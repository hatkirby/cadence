# cadence

`cadence` is a Twitter bot that says things about random songs based on their characteristics.

The data that `cadence` uses to characterize songs comes from the [AcousticBrainz](https://acousticbrainz.org/) project. AcousticBrainz does not currently have an API that would facilitate randomly picking a song, so this bot uses a specially generated datafile that allows it to both randomly choose a song, and identify the type of song it is. Included in this respository is the source of a generator program that can create this datafile from a dump of the AcousticBrainz database.

`cadence` depends on [yaml-cpp](https://github.com/jbeder/yaml-cpp) to read configuration data from a file, SQLite3 to read a datafile, and my own library [libtwitter++](https://github.com/hatkirby/libtwittercpp) to post to Twitter.

The canonical bot, [@songchoicebot](https://twitter.com/songchoicebot), uses the categorization of random songs in order to say things that are ill-fitted to describe the song.
