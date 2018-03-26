#include <iostream>
#include <exception>
#include "generator.h"

void printUsage()
{
  std::cout << "usage: generator input output" << std::endl;
  std::cout << "input  :: path to an AcousticBrainz data directory" << std::endl;
  std::cout << "output :: datafile output path" << std::endl;
}

int main(int argc, char** argv)
{
  if (argc == 3)
  {
    try
    {
      cadence::generator::generator app(argv[1], argv[2]);

      try
      {
        app.run();
      } catch (const std::exception& e)
      {
        std::cout << e.what() << std::endl;
      }
    } catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      printUsage();
    }
  } else {
    std::cout << "cadence datafile generator" << std::endl;
    printUsage();
  }
}
