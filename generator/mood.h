#ifndef MOOD_H_B9A39F40
#define MOOD_H_B9A39F40

#include <string>

namespace cadence {
  namespace generator {
    
    class mood {
    public:
      enum class type {
        danceable,
        acoustic,
        aggressive,
        electronic,
        happy,
        party,
        relaxed,
        sad,
        instrumental
      };
      
      // Constructor
      
      mood(type t, double prob);
      
      // Accessors
      
      type getType() const
      {
        return type_;
      }
      
      double getProbability() const
      {
        return probability_;
      }
      
      bool getPositive() const
      {
        return positive_;
      }
      
      std::string getCategory() const
      {
        return category_;
      }
      
    private:
      type type_;
      double probability_;
      bool positive_;
      std::string category_;
      
    };
    
  };
};

#endif /* end of include guard: MOOD_H_B9A39F40 */
