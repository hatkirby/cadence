#ifndef CADENCE_H_D4AE737D
#define CADENCE_H_D4AE737D

#include <random>
#include <string>
#include <map>
#include <vector>
#include <hkutil/database.h>
#include <twitter.h>

class cadence {
public:

  cadence(
    std::string configFile,
    std::mt19937& rng);

  void run() const;

private:

  std::mt19937& rng_;
  std::map<std::string, std::vector<std::string>> groups_;
  std::unique_ptr<hatkirby::database> database_;
  std::unique_ptr<twitter::client> client_;

};


#endif /* end of include guard: CADENCE_H_D4AE737D */
