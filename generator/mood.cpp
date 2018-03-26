#include "mood.h"

namespace cadence {
  namespace generator {

    // The categories are:
    // - party (+danceable, -acoustic, +electronic, +party) = ~.21
    // - chill (-danceable, +acoustic, -aggressive, -electronic, -party, +relaxed) = ~.49
    // - crazy (+aggressive, -relaxed) = ~.02
    // - happy (+happy, -sad) = ~.009
    // - sad (-happy, +sad) = ~.02
    // - instrumental (+instrumental) = ~.12
    // - vocal (-instrumental) = ~.10

    mood::mood(type t, double prob) : type_(t)
    {
      if (prob >= 0.5)
      {
        probability_ = prob;
        positive_ = true;
      } else {
        probability_ = 1.0 - prob;
        positive_ = false;
      }

      switch (t)
      {
        case type::danceable:
        {
          category_ = (positive_ ? "party" : "chill");

          break;
        }

        case type::acoustic:
        {
          category_ = (positive_ ? "chill" : "party");

          break;
        }

        case type::aggressive:
        {
          category_ = (positive_ ? "crazy" : "chill");

          break;
        }

        case type::electronic:
        {
          category_ = (positive_ ? "party" : "chill");

          break;
        }

        case type::happy:
        {
          category_ = (positive_ ? "happy" : "sad");

          break;
        }

        case type::party:
        {
          category_ = (positive_ ? "party" : "chill");

          break;
        }

        case type::relaxed:
        {
          category_ = (positive_ ? "chill" : "crazy");

          break;
        }

        case type::sad:
        {
          category_ = (positive_ ? "sad" : "happy");

          break;
        }

        case type::instrumental:
        {
          category_ = (positive_ ? "instrumental" : "vocal");

          break;
        }
      }
    }

  };
};
