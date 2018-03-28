CREATE TABLE `songs` (
  `song_id` INTEGER PRIMARY KEY,
  `title` VARCHAR(255) NOT NULL,
  `artist` VARCHAR(255) NOT NULL
);

CREATE TABLE `moods` (
  `song_id` INTEGER,
  `mood` VARCHAR(255),
  PRIMARY KEY (`song_id`, `mood`)
) WITHOUT ROWID;
