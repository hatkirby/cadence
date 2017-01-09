#include "generator.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <json.hpp>
#include "progress.h"
#include "field.h"
#include "../util.h"
#include "mood.h"

namespace cadence {
  namespace generator {
    
    generator::generator(
      std::string inputpath,
      std::string outputpath) :
        inputpath_(inputpath),
        db_(outputpath)
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
      auto queries = split<std::list<std::string>>(schema, ";");
      progress ppgs("Writing database schema...", queries.size());
      for (std::string query : queries)
      {
        if (!queries.empty())
        {
          db_.runQuery(query);
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
            throw std::invalid_argument("Invalid AcousticBrainz data directory");
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
                throw std::invalid_argument("Invalid AcousticBrainz data directory");
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
      progress ppgs("Parsing AcousticBrainz data files...", datafiles_.size());
      
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
          std::vector<mood> moods;
          moods.emplace_back(mood::type::danceable, jsonData["highlevel"]["danceability"]["all"]["danceable"]);
          moods.emplace_back(mood::type::acoustic, jsonData["highlevel"]["mood_acoustic"]["all"]["acoustic"]);
          moods.emplace_back(mood::type::aggressive, jsonData["highlevel"]["mood_aggressive"]["all"]["aggressive"]);
          moods.emplace_back(mood::type::electronic, jsonData["highlevel"]["mood_electronic"]["all"]["electronic"]);
          moods.emplace_back(mood::type::happy, jsonData["highlevel"]["mood_happy"]["all"]["happy"]);
          moods.emplace_back(mood::type::party, jsonData["highlevel"]["mood_party"]["all"]["party"]);
          moods.emplace_back(mood::type::relaxed, jsonData["highlevel"]["mood_relaxed"]["all"]["relaxed"]);
          moods.emplace_back(mood::type::sad, jsonData["highlevel"]["mood_sad"]["all"]["sad"]);
          moods.emplace_back(mood::type::instrumental, jsonData["highlevel"]["voice_instrumental"]["all"]["instrumental"]);
        
          std::sort(std::begin(moods), std::end(moods), [] (const mood& left, const mood& right) -> bool {
            return left.getProbability() > right.getProbability();
          });
        
          std::list<field> fields;
          fields.emplace_back("title", jsonData["metadata"]["tags"]["title"][0].get<std::string>());
          fields.emplace_back("artist", jsonData["metadata"]["tags"]["artist"][0].get<std::string>());
          fields.emplace_back("category", moods.front().getCategory());
        
          db_.insertIntoTable("songs", std::move(fields));
        } catch (const std::domain_error& ex)
        {
          // Weird data. Ignore silently.
        }
      }
    }
    
  };
};
