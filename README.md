# web-cards
This project is a web-server used to play the family card game.

# Dependencies
+ Require at least partial support for C++20.
+ Required boost (beast) ≥ 1.71.

Building this project can easily be done on Ubuntu 20.04 as it have a sufficient GCC version (9.3).

# Use-case
Because of Covid, it's impossible to meet to play a card game. While there exist programs to play card games, nobody know the "official" name of the game we play. As such, it would be really hard to find such an existing program. In addition, it's not everybody that would be able to setup such a program. So I decided to create one.

The major requirements:
+ Nothing to install for the users.
+ Simple UI as some users are less tech-savvy.
+ No password (for joining a game) because it's not everybody that is good at remembering password.
+ Support many resolutions.

Other requirements:
+ Ability to rejoin a game if the original window was closed or minimized.
+ Playing on a tablet must be possible.
