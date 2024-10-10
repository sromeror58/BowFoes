Bow Foes: Game Design Document

# Section 0
Game Title: Bow Foes

Team Members and Roles:

Richard Hoffmann: Gameplay and Controls

Rohun Agrawal: AI and Physics

Steven Romero: Graphics and UI

Concept Statement: 
A 1v1 archery game where players battle through varying levels of difficulty, mastering projectile motion and overcoming physical forces to knock down their opponent first.

# Section 1

Game Progression: 
The game progresses through a series of levels where the player competes against either another player or an AI opponent. Each level increases in difficulty, with the AI becoming increasingly more accurate and harder to hit.

Win and Loss Conditions:
The first player to eliminate the opponent by reducing their health to zero wins
A player loses if their health reaches zero first

Levels:
There are multiple levels, each with unique map designs and increasing difficulty of the AI opponent.

Controls:
Drag mouse to set the angle and power of the shot.

Physics:
We incorporate the physics engine for the arrow projectile motion (influenced by gravity) and collision effects with the players.

Game Flow

Start Screen: 
Players can start a new game, adjust settings, or view instructions.

Gameplay: 
Players take turns aiming and shooting arrows at each other.

End of Turn: 
The turn switches to the other player after each shot.

Win/Loss Screen: 
Displays the result and options to replay or exit.

Graphics:
Sprites for characters and polygons or images for backgrounds and platforms. Might pixel draw images if there is time

# Section 2: Feature Set
Priority 1 Features:

Give each player a health bar that goes down when hit, proportional to the final velocity of the arrow when hit.
We will have turn-based shooting, where the control switches between players with every turn.
Implement arrow shooting mechanics based on user-given input through the terminal - user sets the angle above the ground and power at which projectile is released.
Our basic level design will feature a flat ground with two archers controlled by two users, or one user based on gameplay preference.

Priority 2 Features:

Music feature added to the game.
Implementing drag-and-shoot mechanics through mouse/trackpad to determine direction and magnitude of power of projectile when released.

Priority 3 Features:

One of the players shoots from an elevated platform positioned somewhere in the scene and not on the ground
Make enemy archer an AI that shoots randomly with some predetermined accuracy
Design and implement different maps with varying starting positions of players on varying platform layouts

Priority 4 Features:

Give each level its own background image and ground image
Implement different gravity settings for higher levels like the moon or Mars
The AI accuracy improves with levels
Platforms move during the game at harder levels

Priority 5 Features:

Implement different scenes such as intro scene, level scene and end scene. Also having a retry button will allow for a reattempt at the level.
Allow players to change their character's sprite


# Section 3: Timeline
Week 1

Richard Hoffmann: Health bar/players, turn-based shooting

Rohun Agrawal: Arrow shooting mechanism and music effects

Steven Romero: Basic level design, scene loading (players/archers, background, etc)

Week 2

Richard Hoffmann: Drag-and-shoot capabilities

Rohun Agrawal: AI opponent

Steven Romero: Better graphics for levels

Week 3

Richard Hoffmann: Multiple level layouts, character skins

Rohun Agrawal: AI difficulty scaling

Steven Romero: Variable gravity, moving platforms

# Section 4: Disaster Recovery
Richard Hoffmann

If falling behind, use text indicators until the health bar is functional.
Simplify shooting mechanics if detailed controls are problematic, to just user-input if dragging causes issues.
Start with one detailed level and add more as time permits, such as just one position for stationary platform / moving 
platform if extra time. If falling behind on gameplay, focus on just creating one detailed level with working shooting
mechanisms between 2 in-game characters.

Rohun Agrawal 

If AI development becomes difficult, we could hard code a cycle of shots that achieve a certain accuracy
If falling behind, then graphics can be simplified to changes in colors rather than whole new images.

Steven Romero 

Load all the elements of the scene including the archers and background, if we can’t figure out the hitbox of these elements, use basic shapes for placeholders.
Implement different scenes for different scenarios and if implementing a button to continue to the next scene, we can use user-input through the terminal.
For moving platforms, only focus on linear movement at a slow speed if running into issues with hit registration. 




