#include "generator.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <json.hpp>
#include <hkutil/progress.h>
#include <hkutil/string.h>

namespace cadence {
  namespace generator {

    generator::generator(
      std::string inputpath,
      std::string outputpath) :
        inputpath_(inputpath),
        db_(outputpath, hatkirby::dbmode::create)
    {
      // Add directory separator to input path
      if ((inputpath_.back() != '/') && (inputpath_.back() != '\\'))
      {
        inputpath_ += '/';
      }

      inputpath_ += "highlevel/";
    }

    void generator::run()
    {
      // Creates the datafile.
      writeSchema();

      // Scans the AcousticBrainz data dump and generates a list of all of the
      // files in the dump.
      scanDirectories();

      // Parses each data file and enters it into the database.
      parseData();
    }

    void generator::writeSchema()
    {
      std::ifstream file("schema.sql");
      if (!file)
      {
        throw std::invalid_argument("Could not find database schema");
      }

      std::ostringstream schemaBuilder;
      std::string line;
      while (std::getline(file, line))
      {
        if (line.back() == '\r')
        {
          line.pop_back();
        }

        schemaBuilder << line;
      }

      std::string schema = schemaBuilder.str();
      auto queries = hatkirby::split<std::list<std::string>>(schema, ";");
      hatkirby::progress ppgs("Writing database schema...", queries.size());
      for (std::string query : queries)
      {
        if (!queries.empty())
        {
          db_.execute(query);
        }

        ppgs.update();
      }
    }

    void generator::scanDirectories()
    {
      std::cout << "Scanning AcousticBrainz dump..." << std::endl;

      DIR* topdir;
      if ((topdir = opendir(inputpath_.c_str())) == nullptr)
      {
        throw std::invalid_argument("Invalid AcousticBrainz data directory");
      }

      struct dirent* topent;
      while ((topent = readdir(topdir)) != nullptr)
      {
        if (topent->d_name[0] != '.')
        {
          std::string directory = inputpath_ + topent->d_name + "/";

          DIR* subdir;
          if ((subdir = opendir(directory.c_str())) == nullptr)
          {
            throw std::invalid_argument(
              "Invalid AcousticBrainz data directory");
          }

          struct dirent* subent;
          while ((subent = readdir(subdir)) != nullptr)
          {
            if (subent->d_name[0] != '.')
            {
              std::string subdirectory = directory + subent->d_name + "/";

              DIR* subsubdir;
              if ((subsubdir = opendir(subdirectory.c_str())) == nullptr)
              {
                throw std::invalid_argument(
                  "Invalid AcousticBrainz data directory");
              }

              struct dirent* subsubent;
              while ((subsubent = readdir(subsubdir)) != nullptr)
              {
                if (subsubent->d_name[0] != '.')
                {
                  std::string datafile = subdirectory + subsubent->d_name;

                  datafiles_.push_back(datafile);
                }
              }

              closedir(subsubdir);
            }
          }

          closedir(subdir);
        }
      }

      closedir(topdir);
    }

    void generator::parseData()
    {
      hatkirby::progress ppgs(
        "Parsing AcousticBrainz data files...",
        datafiles_.size());

      for (std::string datafile : datafiles_)
      {
        ppgs.update();

        nlohmann::json jsonData;
        {
          std::ifstream dataStream(datafile);
          dataStream >> jsonData;
        }

        try
        {
          auto& hl = jsonData["highlevel"];

          double danceable = hl["danceability"]["all"]["danceable"];
          double acoustic = hl["mood_acoustic"]["all"]["acoustic"];
          double aggressive = hl["mood_aggressive"]["all"]["aggressive"];
          double electronic = hl["mood_electronic"]["all"]["electronic"];
          double happy = hl["mood_happy"]["all"]["happy"];
          double party = hl["mood_party"]["all"]["party"];
          double relaxed = hl["mood_relaxed"]["all"]["relaxed"];
          double sad = hl["mood_sad"]["all"]["sad"];
          double instrumental = hl["voice_instrumental"]["all"]["instrumental"];

          std::string title = jsonData["metadata"]["tags"]["title"][0];
          std::string artist = jsonData["metadata"]["tags"]["artist"][0];

          uint64_t songId = db_.insertIntoTable(
            "songs",
            {
              { "title", title },
              { "artist", artist }
            });

          std::list<std::string> moods;

          // ~38%
          if ((party > 0.5) || (danceable > 0.75))
          {
            moods.push_back("party");
          }

          // ~38%
          if ((relaxed > 0.81) || (acoustic > 0.5))
          {
            moods.push_back("chill");
          }

          // ~42%
          if ((aggressive > 0.5) || (electronic > 0.95))
          {
            moods.push_back("crazy");
          }

          // ~30%
          if (happy > 0.5)
          {
            moods.push_back("happy");
          }

          // ~30%
          if (sad > 0.5)
          {
            moods.push_back("sad");
          }

          // ~38%
          if (instrumental > 0.9)
          {
            moods.push_back("instrumental");
          }

          // ~34%
          if (instrumental < 0.2)
          {
            moods.push_back("vocal");
          }

          // ~1%
          if (moods.empty())
          {
            moods.push_back("unknown");
          }

          for (const std::string& mood : moods)
          {
            db_.insertIntoTable(
              "moods",
              {
                { "song_id", static_cast<int>(songId) },
                { "mood", mood }
              });
          }
        } catch (const std::domain_error& ex)
        {
          // Weird data. Ignore silently.
        }
      }
    }

  };
};
