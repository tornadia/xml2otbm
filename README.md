# xml2otbm
Transforming data from 7.1 map files to 7.6 otbm readable format

# Currently only loads:
* 7.1 Map (map-v71-to-v772.js)
  * Spawns
  * Portals, Tele, RopeTele, Usetele (the last 3 must be fixed ingame)
  * Monsters (NPC is pending)
  * Level Doors (incomplete)
  * Ground and Items (items might be missing some properties)
* 7.5 OTX (must be loaded as xml, map-otx-to-otbm.js)
  * Not fully implemented, missing a couple of things (spawns and conversion map)
  * Multi-floor
  * Teleports
  * Ground and items (haven't checked for containers yet)

# Pending:
* A bunch of optimizations and fixes
* Convert to library/modularize instead of having such a disaster repo.
* Conversion map from 7.1, 7.5 to 7.72 to make it fully working.
* PZ load from 7.1, also make it 16x16 maparea sector loads instead of 256x1 (not efficient at all)