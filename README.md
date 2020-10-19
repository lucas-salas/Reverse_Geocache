# Reverse_Geocache
This my rendition of a reverse geocache (original idea by Mikal Hart). Instead of going to a location to find a cache, you actually posses the cache and must take it to a location in order for it to open.

reverse_geocache.ino is the main code that runs everything.

In order to retain the number of attempts between poweroffs, the number of attempts is written to the EEPROM. To clear this when the puzzle resets, EEPROM_RESET.ino resets the Arduino's EEPROM.
