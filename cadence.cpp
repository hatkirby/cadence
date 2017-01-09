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
#include "util.h"

const std::vector<std::string> moods = {"party", "chill", "crazy", "happy", "sad", "instrumental", "vocal"};

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
  sqlite3* ppdb = nullptr;
  
  if (sqlite3_open_v2(config["acoustic_datafile"].as<std::string>().c_str(), &ppdb, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
  {
    // We still have to free the resources allocated. In the event that
    // allocation failed, ppdb will be null and sqlite3_close_v2 will just
    // ignore it.
    sqlite3_close_v2(ppdb);

    std::cout << "Could not open acoustic datafile." << std::endl;
    
    return 1;
  }

  for (;;)
  {
    std::cout << "Generating tweet..." << std::endl;
    
    std::string mood = moods[std::uniform_int_distribution<int>(0, moods.size()-1)(random_engine)];
    
    std::string songTitle;
    std::string songArtist;
    
    sqlite3_stmt* ppstmt;
    std::string query = "SELECT title, artist FROM songs WHERE category = ? ORDER BY RANDOM() LIMIT 1";

    if (sqlite3_prepare_v2(ppdb, query.c_str(), query.length(), &ppstmt, NULL) != SQLITE_OK)
    {
      std::cout << "Error reading from acoustic datafile:" << std::endl;
      std::cout << sqlite3_errmsg(ppdb) << std::endl;
      
      return 1;
    }
    
    sqlite3_bind_text(ppstmt, 1, mood.c_str(), mood.length(), SQLITE_TRANSIENT);
    
    if (sqlite3_step(ppstmt) == SQLITE_ROW)
    {
      songTitle = reinterpret_cast<const char*>(sqlite3_column_blob(ppstmt, 0));
      songArtist = reinterpret_cast<const char*>(sqlite3_column_blob(ppstmt, 1));
    } else {
      std::cout << "Error reading from acoustic datafile:" << std::endl;
      std::cout << sqlite3_errmsg(ppdb) << std::endl;
      
      return 1;
    }
    
    sqlite3_finalize(ppstmt);

    std::string action = "{" + cadence::uppercase(mood) + "}";
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
      
      std::string canontkn;
      std::transform(std::begin(token), std::end(token), std::back_inserter(canontkn), [] (char ch) {
        return std::toupper(ch);
      });
      
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
        std::cout << "Badly formatted forms file:" << std::endl;
        std::cout << "No such form as " + canontkn << std::endl;
        
        return 1;
      }
      
      if (modifier == "indefinite")
      {
        if ((result.length() > 1) && (isupper(result[0])) && (isupper(result[1])))
        {
          result = "an " + result;
        } else if ((result[0] == 'a') || (result[0] == 'e') || (result[0] == 'i') || (result[0] == 'o') || (result[0] == 'u'))
        {
          result = "an " + result;
        } else {
          result = "a " + result;
        }
      }
      
      std::string finalresult;
      if (islower(token[0]))
      {
        std::transform(std::begin(result), std::end(result), std::back_inserter(finalresult), [] (char ch) {
          return std::tolower(ch);
        });
      } else if (isupper(token[0]) && !isupper(token[1]))
      {
        auto words = cadence::split<std::list<std::string>>(result, " ");
        for (auto& word : words)
        {
          word[0] = std::toupper(word[0]);
        }
        
        finalresult = cadence::implode(std::begin(words), std::end(words), " ");
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
