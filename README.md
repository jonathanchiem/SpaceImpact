# Space Impact

### Table of Contents

> [Introduction](#introduction)

> [Hardware](#hardware)

> [Parts List](#parts-list)

> [Pinout](#pinout)

> [Software](#software)

> [Complexities](#complexities)

> [Youtube Link](#youtube-link)

> [Known Bugs and Shortcomings](#known-bugs-and-shortcomings)

> [Future work](#future-work)

> [Citations](#citations)

## Introduction
Space Impact is a two-dimensional space battle game. The player controls a spaceship that moves vertically and horizontally through the level shooting missiles at upcoming enemies. There is ammunition bar that keeps track of how many bullets the player currently has. If the bar is below maximum capacity, it refills one bullet every second. Every enemy destroyed credits the player with one point. The points scored is displayed once the game is over. The high score is updated when a game is over, and is displayed on the start menu. The game is over when the player collides with an enemy, or an enemy collides with the player. The game can also be reset at any point, which returns the player to the start menu.

<img src = "https://user-images.githubusercontent.com/42078356/47338968-92979180-d64e-11e8-9ffd-02b0fd71fbcb.JPG" width="400"> <img src = "https://user-images.githubusercontent.com/42078356/47338976-975c4580-d64e-11e8-9568-3cae20d17712.JPG" width="400">

## Hardware
### Parts List
- ATMega1284p microcontroller
- Shift register
- 10-LED bar
- Buttons
- SNES Controller
- LCD Screen


### Pinout
![spaceimpactpinout](https://user-images.githubusercontent.com/42078356/47338963-8f9ca100-d64e-11e8-8292-a45b6306403f.png)

### Software
![spaceimpactblockdiagrams](https://user-images.githubusercontent.com/42078356/47338757-f79eb780-d64d-11e8-82fd-68a54a01dbcb.png)

When the game turns on, the start menu and high score are shown. If B5 (start button) is pushed, then the game transitions to a playing state, moving all the other state machines from their initial state. Ship spawning spawns the ship when start is flagged. Ship movement goes to a wait state that waits for a button input when start is flagged. Ship firing goes to a wait state that waits for an input from B4 (shoot button), and outputs if the first or second enemy is hit and the amount of ammo to the shift register to display on the LED bar. Enemy 1 and 2 both spawn the enemy and begin the enemies’ movement patterns. If either enemy is hit, the corresponding state machine is put into a respawn state, and continues. If the player collides with an enemy a lose flag is triggered, which brings the first state machine to a gameover state that checks if the score is greater than the high score; if true, then the score is saved as the new high score in EEPROM.

## Complexities
### Completed Complexities:
- Using shift register with LED bar to display ammunition
- Using EEPROM to save the high score (enemies killed)
- Creating custom characters on the LCD screen
- Game logic(movement, shooting, reloading, collision detection, bullet hit detection, score)

### Incomplete complexities:
- SNES controller
- Game logic(enemy spawning)

## [Youtube Link](https://youtu.be/Rr9_ZhVtqmo)

## Known Bugs and Shortcomings
- Before starting the game, all but two of the LEDs on the LED bar are illuminated, or none of them are. Once the game starts they become fully illuminated.
- Due to hit detection being dysfunctional, the score cannot be kept properly, and for EEPROM demonstration purposes the amount of ammo is displayed instead. In the shooting animation state machine, there is a condition that checks if the position of the bullet and enemy are in the same location and increments score, but the score would not go beyond one because the enemy spawn/movement state machine has a bug.
- When the high score is above nine , the score is not displayed properly. It usually shows a semicolon when it should be ten.
- After soft resetting, the reload feature continues to fill the bar, which updates the EEPROM; though storing the amount of ammo is not the intended purpose of EEPROM.
- After the first pass across the LCD, the enemy does not respawn. The enemy spawning state machine is most likely not returning to the spawning state properly.
- After soft resetting, sometime reappears in front of the player, other times it will not. This is also likely due to the initializing and spawning states.
- Implementation of SNES controller was completed, but was not able to be utilized due to a faulty SNES controller. I was able to confirm this when testing the controller on a peer’s project that had a working SNES controller.

## Future work
- Larger LCD screen such as Nokia 5110
- Power-ups, such as bonus health (increasing health beyond one), multi-shot (shoots multiple bullets at once in different rows)
- Boss battle after x time elapsed
- Multiple stages

## Citations

### [SNES Protocol](http://uzebox.org/files/NES-controller-Hydra-Ch6All-v1.0.pdf)

### EEPROM - [“Using the EEPROM in AVR-GCC”](http://www.fourwalledcubicle.com/AVRArticles.php)

