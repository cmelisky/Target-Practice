# Final Project - Target Practice

# Author
Chase Melisky

# Installations
- C++ compiler (supporting C++11 or later)
- GLFW (OpenGL library)
- GLAD (OpenGL loader)

# Summary
This game employs lots of moving targets and navigation between screens to create a fun target practice. The user clicks to shoot at a moving object and destroys it. Each level has different movement patterns, colors, and win conditions, and the program tracks the user's accuracy. As you hit more targets, you accumulate points to eventually win. The end screen shows where you can move to the next level, or give the one you just played another go. There are even two modes, and the hardmode makes the targets move quicker and more erratically. I hope you enjoy!

# Known Bugs
No known bugs to report.

# Future Work
Ideally, this project would've been complete with a bonusBox. BonusBox was a part of the game that would send a yellow target across the screen, giving the player 5 points. Unfortunately it kind of broke the program, rendering it useless, so I commented out that code. However, I believe I can work at it in the future and make it work properly. 

# Citations
- Foundation code from M4OEP by myself, Ian Cox, and Ben Calhoun

# Concepts Used
- Key input
- Mouse input
- Multiple Screens in same graphics window
- Non-input based events


# Grade Expectation

- Engine program complexity and usability: My engine.cpp file is quite complex, using a lot of different logic to make the different levels operate. It's well organized, has clear functionality, and is using plenty of concepts from modules throughout the class. (55 pts)
- Interactivity of program: The complexity of user input is simple. A start screen prompts user to press 's' to start the game, and as they click on the targets, their score is incremented. After beating a level, the user can navigate across screens by pressing 'r' to replay the level they just played, or the number corresponding to the level they want to play. (20 pts)
- User experience: I believe my program is very user-friendly, as it states clear instructions and a start key on the start screen. The goal of the game is simple and it is easily replayable. (20 pts)
- Keyboard and mouse input: The keyboard and mouse input is intuitive and responsive, and instructions for all code are clearly given. (20 pts)
- Variety of screens: My program has 4 screens, all with different colors and functionality that can be changed based on the mode of the game. You can move between them, or repeat, from each level. (15)
- Non-input based events:  My NIB event, bonusBox, did not come out how I wanted it to. It would have appeared without any user input necessary, but unfortunately it broke my whole program, so I had to get rid of it. (0 pts)
- Code organization: My code is organized well given the scope and complexity of my project, and it's clearly segmented into functions. (15 pts)
- Style and documentation: My code is well documented and easy to follow, and it details the more complex functionality as well as where things were commented out for the bonusBox. (20 pts)
  
Self-given grade: 150










