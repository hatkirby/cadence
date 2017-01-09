#ifndef GENERATOR_H_B180515E
#define GENERATOR_H_B180515E

#include <string>
#include <list>
#include "database.h"

namespace cadence {
  namespace generator {
    
    class generator {
    public:
      
      // Constructor
      
      generator(
        std::string inputpath,
        std::string outputpath);
        
      // Action
        
      void run();
      
      // Subroutines
      
      void writeSchema();
      
      void scanDirectories();
      
      void parseData();
      
    private:
      
      // Input
      
      std::string inputpath_;
      
      // Output
      
      database db_;
      
      // Cache
      
      std::list<std::string> datafiles_;
      
    };
    
  };
};

#endif /* end of include guard: GENERATOR_H_B180515E */
