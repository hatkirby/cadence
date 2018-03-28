#include <stdexcept>
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <twitter.h>
#include <chrono>
#include <thread>
#include <random>
#include <list>
#include <hkutil/string.h>
#include <hkutil/database.h>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: cadence [configfile]" << std::endl;
    return -1;
  }

  std::string configfile(argv[1]);
  YAML::Node config = YAML::LoadFile(configfile);

  // Set up the Twitter client.
  twitter::auth auth;
  auth.setConsumerKey(config["consumer_key"].as<std::string>());
  auth.setConsumerSecret(config["consumer_secret"].as<std::string>());
  auth.setAccessKey(config["access_key"].as<std::string>());
  auth.setAccessSecret(config["access_secret"].as<std::string>());

  twitter::client client(auth);

  // Read in the forms file.
  std::map<std::string, std::vector<std::string>> groups;
  std::ifstream datafile(config["forms"].as<std::string>());
  if (!datafile.is_open())
  {
    std::cout << "Could not find forms file." << std::endl;
    return 1;
  }

  bool newgroup = true;
  std::string line;
  std::string curgroup;
  while (getline(datafile, line))
  {
    if (line.back() == '\r')
    {
      line.pop_back();
    }

    if (newgroup)
    {
      curgroup = line;
      newgroup = false;
    } else {
      if (line.empty())
      {
        newgroup = true;
      } else {
        groups[curgroup].push_back(line);
      }
    }
  }

  // Initialize the random number generator.
  std::random_device random_device;
  std::mt19937 random_engine{random_device()};

  // Connect to the AcousticBrainz data file.
  hatkirby::database abdb(
    config["acoustic_datafile"].as<std::string>(),
    hatkirby::dbmode::read);

  for (;;)
  {
    std::cout << "Generating tweet..." << std::endl;

    hatkirby::row songRow =
      abdb.queryFirst(
        "SELECT song_id, title, artist FROM songs ORDER BY RANDOM() LIMIT 1");

    hatkirby::row moodRow =
      abdb.queryFirst(
        "SELECT mood FROM moods WHERE song_id = ? ORDER BY RANDOM() LIMIT 1",
        { mpark::get<int>(songRow[0]) });

    std::string songTitle = mpark::get<std::string>(songRow[1]);
    std::string songArtist = mpark::get<std::string>(songRow[2]);
    std::string mood = mpark::get<std::string>(moodRow[0]);

    std::string action = "{" + hatkirby::uppercase(mood) + "}";
    int tknloc;
    while ((tknloc = action.find("{")) != std::string::npos)
    {
      std::string token = action.substr(tknloc+1, action.find("}")-tknloc-1);

      std::string modifier;
      int modloc;
      if ((modloc = token.find(":")) != std::string::npos)
      {
        modifier = token.substr(modloc+1);
        token = token.substr(0, modloc);
      }

      std::string canontkn = hatkirby::uppercase(token);

      std::string result;
      if (canontkn == "\\N")
      {
        result = "\n";
      } else if (canontkn == "ARTIST")
      {
        result = songArtist;
      } else if (canontkn == "SONG")
      {
        result = "\"" + songTitle + "\"";
      } else if (groups.count(canontkn)) {
        auto& group = groups.at(canontkn);
        std::uniform_int_distribution<int> dist(0, group.size() - 1);

        result = group[dist(random_engine)];
      } else {
        throw std::logic_error("No such form as " + canontkn);
      }

      if (modifier == "indefinite")
      {
        if (
          (result.length() > 1) &&
          isupper(result[0]) &&
          isupper(result[1]))
        {
          result = "an " + result;
        } else if (
          (result[0] == 'a') ||
          (result[0] == 'e') ||
          (result[0] == 'i') ||
          (result[0] == 'o') ||
          (result[0] == 'u'))
        {
          result = "an " + result;
        } else {
          result = "a " + result;
        }
      }

      std::string finalresult;
      if (islower(token[0]))
      {
        finalresult = hatkirby::lowercase(result);
      } else if (isupper(token[0]) && !isupper(token[1]))
      {
        auto words = hatkirby::split<std::list<std::string>>(result, " ");
        for (auto& word : words)
        {
          word[0] = std::toupper(word[0]);
        }

        finalresult = hatkirby::implode(
          std::begin(words),
          std::end(words),
          " ");
      } else {
        finalresult = result;
      }

      action.replace(tknloc, action.find("}")-tknloc+1, finalresult);
    }

    if (action.size() <= 140)
    {
      try
      {
        client.updateStatus(action);

        std::cout << action << std::endl;
      } catch (const twitter::twitter_error& e)
      {
        std::cout << "Twitter error: " << e.what() << std::endl;
      }

      std::cout << "Waiting..." << std::endl;

      std::this_thread::sleep_for(std::chrono::hours(1));

      std::cout << std::endl;
    } else {
      std::cout << "Tweet was too long, regenerating." << std::endl;
    }
  }
}
