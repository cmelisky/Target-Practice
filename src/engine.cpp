#include "engine.h"
#include <iostream>
#include <cstdlib>
#include <sstream>

enum state {start, level1, level2, level3, level4, over};
state screen;

//Tracker-related variables
bool gameStarted = false;
int startTime = 0;
int endTime = 0;
int clicks = 0;
bool hardMode = false;
string hard = "";
string currentLevel = "1";

//Accuracy variables
double shotsTaken = 0;
double shotsHit = 0;
double accuracy = 0.0;
int score = 0;

//Colors
const color skyBlue(77/255.0, 213/255.0, 240/255.0);
const color grassGreen(26/255.0, 176/255.0, 56/255.0);
const color darkGreen(27/255.0, 81/255.0, 45/255.0);
const color white(1, 1, 1);
const color brickRed(201/255.0, 20/255.0, 20/255.0);
const color darkBlue(1/255.0, 110/255.0, 214/255.0);
const color purple(119/255.0, 11/255.0, 224/255.0);
const color grey(0.5, 0.5, 0.5);
const color black(0, 0, 0);
const color magenta(1, 0, 1);
const color orange(1, 163/255.0, 22/255.0);
const color cyan (0, 1, 1);
const color yellow (1, 1, 0);
const color gold (238/255.0, 232/255.0, 170/255.0);

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms that never change
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    //user is a 10x10 white block centered at 0,0
    user = make_unique<Rect>(shapeShader, vec2(0, 0), vec2(10, 10), white); // placeholder for compilation

    //bonusBox is a 35x35 yellow box that flies across the screen
    bonusBox = make_unique<Rect>(shapeShader, vec2(-20, 300), vec2(35, 35), yellow); // placeholder for compilation

    // Init level1 background
    grass1 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height * 2), black);
    bottomBorder1 = make_unique<Rect>(shapeShader, vec2(width/2, 0), vec2(width, height/3), grey);
    topBorder1 = make_unique<Rect>(shapeShader, vec2(width/2, 600), vec2(width, height/3), grey);
    
    // Init level2 background
    grass2 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height * 2), grey);
    bottomBorder2 = make_unique<Rect>(shapeShader, vec2(width/2, 0), vec2(width, height/3), black);
    topBorder2 = make_unique<Rect>(shapeShader, vec2(width/2, 600), vec2(width, height/3), black);

    // Init level3 background
    grass3 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height * 2), darkGreen);
    bottomBorder3 = make_unique<Rect>(shapeShader, vec2(width/2, 0), vec2(width, height/3), grassGreen);
    topBorder3 = make_unique<Rect>(shapeShader, vec2(width/2, 600), vec2(width, height/3), grassGreen);

    // Init level4 background
    grass4 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height * 2), grassGreen);
    bottomBorder4 = make_unique<Rect>(shapeShader, vec2(width/2, 0), vec2(width, height/3), darkGreen);
    topBorder4 = make_unique<Rect>(shapeShader, vec2(width/2, 600), vec2(width, height/3), darkGreen);

    // Init targets from closest to furthest
    int totalTargetWidth = 0;
    vec2 targetSize;
    while (totalTargetWidth < width + 50) {
        // Target height between 50-100
        targetSize.y = rand() % 31 + 30;
        // Target width between 30-50
        targetSize.x = rand() % 31 + 30;
        targets1.push_back(make_unique<Rect>(shapeShader,
                                             vec2(totalTargetWidth + (targetSize.x / 2.0) + 20,
                                                  rand() % 200 + 200),
                                             targetSize, brickRed));
        totalTargetWidth += targetSize.x + 5;
    }
    // Populate second set of targets
    totalTargetWidth = 0;
    while (totalTargetWidth < width + 100) {
        // Target height between 100-200
        targetSize.y = rand() % 41 + 40;
        // Target width between 50-100
        targetSize.x = rand() % 41 + 40;
        // Populating vector of darkBlue targets
        targets2.push_back(make_unique<Rect>(shapeShader,
                                             vec2(totalTargetWidth + (targetSize.x / 2.0) + 20,
                                                  rand() % 200 + 200),
                                             targetSize, darkBlue));
        totalTargetWidth += targetSize.x + 5;
    }
    // Populate third set of targets
    totalTargetWidth = 0;
    while (totalTargetWidth < width + 200) {
        // Target height between 200-400
        targetSize.y = rand() % 61 + 60;
        // Target width between 100-200
        targetSize.x = rand() % 61 + 60;
        // Populating vector of purple targets
        targets3.push_back(make_unique<Rect>(shapeShader,
                                             vec2(totalTargetWidth + (targetSize.x / 2.0) + 20,
                                                  rand() % 200 + 200),
                                             targetSize, purple));
        totalTargetWidth += targetSize.x + 5;
    }
}


void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // User moving with the mouse
    user->setPosX(MouseX);
    user->setPosY(MouseY);

    //Starts timer
    if (screen == start && keys[GLFW_KEY_S]) {
        screen = level1;
        gameStarted = true;
        startTime = glfwGetTime();
    }

    // Level 1 Controls
    if (screen == level1) {
        currentLevel = "1";
        shotsTaken = 0;
        shotsHit = 0;
        accuracy = 0;

        if (mousePressedLastFrame && !mousePressed) {
            clicks++;
            shotsTaken++;
        }

        if (bonusBox->isOverlapping(*user)) {
            bonusBox->setColor(gold);
            if (mousePressedLastFrame && !mousePressed) {
                shotsTaken++;
                if (bonusBox->isOverlapping(*user)) {
                    shotsHit++;
                    bonusBox->moveY(-600);
                }
            }
        } else {
            bonusBox->setColor(yellow);
        }
        if (bonusBox->getTop() < 10) {
            bonusBox->setPosX(-20);
            score + 5;
        }

        for (const unique_ptr<Rect>& r : targets1) {
            if (r->isOverlapping(*user)) {
                r->setColor(orange);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsHit++;
                    r->moveY(-600);
                }
            } else {
                r->setColor(brickRed);
            }
            if (r->getTop() < 10) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosY(590);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets2) {
            if (r->isOverlapping(*user)) {
                r->setColor(cyan);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsHit++;
                    r->moveY(-600);
                }
            } else {
                r->setColor(darkBlue);
            }
            if (r->getTop() < 10) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosY(590);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets3) {
            if (r->isOverlapping(*user)) {
                r->setColor(magenta);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsHit++;
                    r->moveY(-600);
                }
            } else {
                r->setColor(purple);
            }
            if (r->getTop() < 10) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosY(590);
                r->setColor(white);
                score++;
            }
        }
        if (score > 37 || keys[GLFW_KEY_G]) {
            accuracy = 100.0 * shotsHit / shotsTaken;
            screen = over;
        }
    }

    // Level 2 controls
    if (screen == level2) {
        currentLevel = "2";
        if (mousePressedLastFrame && !mousePressed) {
            clicks++;
        }

        if (bonusBox->isOverlapping(*user)) {
            bonusBox->setColor(gold);
            if (mousePressedLastFrame && !mousePressed) {
                shotsTaken++;
                if (bonusBox->isOverlapping(*user)) {
                    shotsHit++;
                    bonusBox->moveX(1000);
                }
            }
        } else {
            bonusBox->setColor(yellow);
        }
        if (bonusBox->getLeft() > 900) {
            bonusBox->setPosX(-20);
            score + 5;
        }

        for (const unique_ptr<Rect>& r : targets1) {
            if (r->isOverlapping(*user)) {
                r->setColor(orange);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveX(1000);
                    }
                }
            } else {
                r->setColor(brickRed);
            }
            if (r->getLeft() > 900) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosX(-50);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets2) {
            if (r->isOverlapping(*user)) {
                r->setColor(cyan);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveX(1000);
                    }
                }
            } else {
                r->setColor(darkBlue);
            }
            if (r->getLeft() > 900) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosX(-50);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets3) {
            if (r->isOverlapping(*user)) {
                r->setColor(magenta);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveX(1000);
                    }
                }
            } else {
                r->setColor(purple);
            }
            if (r->getLeft() > 900) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosX(-50);
                r->setColor(white);
                score++;
            }
        }
        if (score >= 34 || keys[GLFW_KEY_G]) {
            accuracy = 100.0 * shotsHit / shotsTaken;
            screen = over;
        }
    }

    // Level 3 controls
    if (screen == level3) {
        currentLevel = "3";
        if (mousePressedLastFrame && !mousePressed) {
            clicks++;
        }

        if (bonusBox->isOverlapping(*user)) {
            bonusBox->setColor(gold);
            if (mousePressedLastFrame && !mousePressed) {
                shotsTaken++;
                if (bonusBox->isOverlapping(*user)) {
                    shotsHit++;
                    bonusBox->moveY(700);
                }
            }
        } else {
            bonusBox->setColor(yellow);
        }
        if (bonusBox->getBottom() < 600) {
            bonusBox->setPosX(-20);
            score + 5;
        }

        for (const unique_ptr<Rect>& r : targets1) {
            if (r->isOverlapping(*user)) {
                r->setColor(orange);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveY(700);
                    }
                }
            } else {
                r->setColor(brickRed);
            }
            if (r->getBottom() > 600) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosY(-50);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets2) {
            if (r->isOverlapping(*user)) {
                r->setColor(cyan);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveY(700);
                    }
                }
            } else {
                r->setColor(darkBlue);
            }
            if (r->getBottom() > 600) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosY(-50);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets3) {
            if (r->isOverlapping(*user)) {
                r->setColor(magenta);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveY(700);
                    }
                }
            } else {
                r->setColor(purple);
            }
            if (r->getBottom() > 600) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosY(-50);
                r->setColor(white);
                score++;
            }
        }
        if (score >= 37 || keys[GLFW_KEY_G]) {
            accuracy = 100.0 * shotsHit / shotsTaken;
            screen = over;
        }
    }

    // Level 4 controls
    if (screen == level4) {
        currentLevel = "4";
        if (mousePressedLastFrame && !mousePressed) {
            clicks++;
        }

        if (bonusBox->isOverlapping(*user)) {
            bonusBox->setColor(gold);
            if (mousePressedLastFrame && !mousePressed) {
                shotsTaken++;
                if (bonusBox->isOverlapping(*user)) {
                    shotsHit++;
                    bonusBox->moveX(-900);
                }
            }
        } else {
            bonusBox->setColor(yellow);
        }
        if (bonusBox->getRight() < -75) {
            bonusBox->setPosX(-20);
            score + 5;
        }

        for (const unique_ptr<Rect>& r : targets1) {
            if (r->isOverlapping(*user)) {
                r->setColor(orange);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveX(-800);
                    }
                }
            } else {
                r->setColor(brickRed);
            }
            if (r->getRight() < -50) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosX(900);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets2) {
            if (r->isOverlapping(*user)) {
                r->setColor(cyan);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveX(-800);
                    }
                }
            } else {
                r->setColor(darkBlue);
            }
            if (r->getRight() < -50) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosX(900);
                r->setColor(white);
                score++;
            }
        }

        for (const unique_ptr<Rect>& r : targets3) {
            if (r->isOverlapping(*user)) {
                r->setColor(magenta);
                if (mousePressedLastFrame && !mousePressed) {
                    shotsTaken++;
                    if (r->isOverlapping(*user)) {
                        shotsHit++;
                        r->moveX(-800);
                    }
                }
            } else {
                r->setColor(purple);
            }
            if (r->getRight() < -50) {
                r->setSizeX(5);
                r->setSizeY(5);
                r->setPosX(900);
                r->setColor(white);
                score++;
            }
        }
        if (score >= 37 || keys[GLFW_KEY_G]) {
            accuracy = 100.0 * shotsHit / shotsTaken;
            screen = over;
        }
    }

    //restart function
    if (screen == over) {
        if (keys[GLFW_KEY_H]) {
            hardMode = true;
        }
        if (keys[GLFW_KEY_N]) {
            hardMode = false;
        }
        if (keys[GLFW_KEY_R]) {
            if (currentLevel == "1") {
                targets1.clear();
                targets2.clear();
                targets3.clear();
                this->initShapes();
                score = 0;
                shotsTaken = 0;
                shotsHit = 0;
                clicks = 0;
                screen = level1;
            } else if (currentLevel == "2") {
                targets1.clear();
                targets2.clear();
                targets3.clear();
                this->initShapes();
                score = 0;
                shotsTaken = 0;
                shotsHit = 0;
                clicks = 0;
                screen = level2;
            } else if (currentLevel == "3") {
                targets1.clear();
                targets2.clear();
                targets3.clear();
                this->initShapes();
                score = 0;
                shotsTaken = 0;
                shotsHit = 0;
                clicks = 0;
                screen = level3;
            } else if (currentLevel == "4") {
                targets1.clear();
                targets2.clear();
                targets3.clear();
                this->initShapes();
                score = 0;
                shotsTaken = 0;
                shotsHit = 0;
                clicks = 0;
                screen = level4;
            } else {
                screen = level1;
            }
        }
        if (keys[GLFW_KEY_1]) {
            targets1.clear();
            targets2.clear();
            targets3.clear();
            this->initShapes();
//            bonusBox->setPosX(-20);
//            bonusBox->setPosY(300);
            score = 0;
            shotsTaken = 0;
            shotsHit = 0;
            clicks = 0;
            screen = level1;
        }
        if (keys[GLFW_KEY_2]) {
            targets1.clear();
            targets2.clear();
            targets3.clear();
            this->initShapes();
//            bonusBox->setPosX(400);
//            bonusBox->setPosY(620);
            score = 0;
            shotsTaken = 0;
            shotsHit = 0;
            clicks = 0;
            screen = level2;
        }
        if (keys[GLFW_KEY_3]) {
            targets1.clear();
            targets2.clear();
            targets3.clear();
            this->initShapes();
            //bonusBox->setPosX(-20);
            score = 0;
            shotsTaken = 0;
            shotsHit = 0;
            clicks = 0;
            screen = level3;
        }
        if (keys[GLFW_KEY_4]) {
            targets1.clear();
            targets2.clear();
            targets3.clear();
            this->initShapes();
            //bonusBox->setPosX(-20);
            score = 0;
            shotsTaken = 0;
            shotsHit = 0;
            clicks = 0;
            screen = level4;
        }
    }

    if (hardMode == true) {
        hard = "hard";
    } else {
        hard = "normal";
    }

    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (gameStarted && screen == over && endTime == 0) {
        endTime = currentFrame;
    }

    // Update targets (level 1)
    if (screen == level1) {
//        if (score > 9 && score < 12) {
//            if (hardMode == false) {
//                bonusBox->moveX(3);
//            }
//            if (hardMode == true) {
//                bonusBox->moveX(-6);
//            }
//            if (bonusBox->getLeft() < 825) {
//                bonusBox->setPosX(-20);
//            }
//        }
        for (int i = 0; i < targets1.size(); ++i) {
            // Move all the red targets to the left
            if (hardMode == false) {
                targets1[i]->moveX(-1.5);
            }
            if (hardMode == true) {
                targets1[i]->moveX(-4);
            }
            // If a target has moved off the screen
            if (targets1[i]->getPosX() < -(targets1[i]->getSize().x / 2)) {
                // Set it to the right of the screen so that it passes through again
                int targetOnLeft = (targets1[i] == targets1[0]) ? targets1.size() - 1 : i - 1;
                targets1[i]->setPosX(targets1[targetOnLeft]->getPosX() + targets1[targetOnLeft]->getSize().x / 2 +
                                     targets1[i]->getSize().x / 2 + 5);
            }
        }

        for (int i = 0; i < targets2.size(); ++i) {
            // Move all the blue targets to the left
            if (hardMode == false) {
                targets2[i]->moveX(3.0);
            }
            if (hardMode == true) {
                targets2[i]->moveX(1.5);
            }
            // If a target has moved off the screen
            if (targets2[i]->getPosX() > width + (targets2[i]->getSize().x / 2)) {
                // Regenerate on the right of the screen
                int targetOnRight = (targets2[i] == targets2[targets2.size() - 1]) ? 0 : i + 1;
                targets2[i]->setPosX(targets2[targetOnRight]->getPosX() - targets2[targetOnRight]->getSize().x / 2 -
                                     targets2[i]->getSize().x / 2 - 5);
            }
        }

        for (int i = 0; i < targets3.size(); ++i) {
            // Move all the purple targets to the left
            if (hardMode == false) {
                targets3[i]->moveX(-.5);
            }
            if (hardMode == true) {
                targets3[i]->moveX(-3);
            }
            // If a target has moved off the screen
            if (targets3[i]->getPosX() < -(targets3[i]->getSize().x / 2)) {
                // Set it to the right of the screen so that it passes through again
                int targetOnLeft = (targets3[i] == targets3[0]) ? targets3.size() - 1 : i - 1;
                targets3[i]->setPosX(targets3[targetOnLeft]->getPosX() + targets3[targetOnLeft]->getSize().x / 2 +
                                     targets3[i]->getSize().x / 2 + 5);
            }
        }
    }

    // Update targets (level 2)
    if (screen == level2) {
//        if (score > 9 && score < 12) {
//            if (hardMode == false) {
//                bonusBox->moveY(-3);
//            }
//            if (hardMode == true) {
//                bonusBox->moveY(-6);
//            }
//            if (bonusBox->getTop() < 10) {
//                bonusBox->setPosY(600);
//            }
//        }
        for (int i = 0; i < targets1.size(); ++i) {
            // Move all the red targets upwards
            if (hardMode == false) {
                targets1[i]->moveY(-1.5);
            }
            if (hardMode == true) {
                targets1[i]->moveY(-3);
            }
            // If a target has moved off the screen
            if (targets1[i]->getPosY() < -(targets1[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets1[i] == targets1[0]) ? targets1.size() - 1 : i - 1;
                targets1[i]->setPosY(600);
            }
        }

        for (int i = 0; i < targets2.size(); ++i) {
            // Move all the red targets upwards
            if (hardMode == false) {
                targets2[i]->moveY(-3);
            }
            if (hardMode == true) {
                targets2[i]->moveY(-5);
            }
            // If a target has moved off the screen
            if (targets2[i]->getPosY() < -(targets2[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets2[i] == targets2[0]) ? targets2.size() - 1 : i - 1;
                targets2[i]->setPosY(600);
            }
        }

        for (int i = 0; i < targets3.size(); ++i) {
            // Move all the red targets upwards
            if (hardMode == false) {
                targets3[i]->moveY(-.5);
            }
            if (hardMode == true) {
                targets3[i]->moveY(-4);
            }
            // If a target has moved off the screen
            if (targets3[i]->getPosY() < -(targets3[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets3[i] == targets3[0]) ? targets3.size() - 1 : i - 1;
                targets3[i]->setPosY(600);
            }
        }
    }

    //Update targets (level 3)
    if (screen == level3) {
//        if (score > 9 && score < 12) {
//            if (hardMode == false) {
//                bonusBox->moveX(-3);
//                bonusBox->moveY(-3);
//            }
//            if (hardMode == true) {
//                bonusBox->moveX(-6);
//                bonusBox->moveY(-6);
//            }
//
//        }
        for (int i = 0; i < targets1.size(); ++i) {
            // Move all the red targets diagonally
            if (hardMode == false) {
                targets1[i]->moveX(-1.5);
                targets1[i]->moveY(-1.5);
            }
            if (hardMode == true) {
                targets1[i]->moveX(-3);
                targets1[i]->moveY(-3);
            }
            // If a target has moved off the screen
            if (targets1[i]->getPosX() < -(targets1[i]->getSize().x / 2)) {
                // Set it to the right of the screen so that it passes through again
                int targetOnLeft = (targets1[i] == targets1[0]) ? targets1.size() - 1 : i - 1;
                targets1[i]->setPosX(790);
            }
            if (targets1[i]->getPosY() < -(targets1[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets1[i] == targets1[0]) ? targets1.size() - 1 : i - 1;
                targets1[i]->setPosY(590);
            }
        }

        for (int i = 0; i < targets2.size(); ++i) {
            // Move all the blue targets diagonally
            if (hardMode == false) {
                targets2[i]->moveX(-3);
                targets2[i]->moveY(-3);
            }
            if (hardMode == true) {
                targets2[i]->moveX(-4.5);
                targets2[i]->moveY(-4.5);
            }
            // If a target has moved off the screen
            if (targets2[i]->getPosX() < -(targets2[i]->getSize().x / 2)) {
                // Set it to the right of the screen so that it passes through again
                int targetOnLeft = (targets2[i] == targets2[0]) ? targets2.size() - 1 : i - 1;
                targets2[i]->setPosX(790);
            }
            if (targets2[i]->getPosY() < -(targets2[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets2[i] == targets2[0]) ? targets2.size() - 1 : i - 1;
                targets2[i]->setPosY(590);
            }
        }

        for (int i = 0; i < targets3.size(); ++i) {
            // Move all the purple targets diagonally
            if (hardMode == false) {
                targets3[i]->moveX(-.5);
                targets3[i]->moveY(-.5);
            }
            if (hardMode == true) {
                targets3[i]->moveX(-2);
                targets3[i]->moveY(-2);
            }
            // If a target has moved off the screen
            if (targets3[i]->getPosX() < -(targets3[i]->getSize().x / 2)) {
                // Set it to the right of the screen so that it passes through again
                int targetOnLeft = (targets3[i] == targets3[0]) ? targets3.size() - 1 : i - 1;
                targets3[i]->setPosX(790);
            }
            if (targets3[i]->getPosY() < -(targets3[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets3[i] == targets3[0]) ? targets3.size() - 1 : i - 1;
                targets3[i]->setPosY(590);
            }
        }
    }

    //Update screen (level4)
    if (screen == level4) {
//        if (score > 9 && score < 12) {
//            if (hardMode == false) {
//                bonusBox->moveX(3);
//                bonusBox->moveY(-3);
//            }
//            if (hardMode == true) {
//                bonusBox->moveX(6);
//                bonusBox->moveY(-6);
//            }
//        }
        for (int i = 0; i < targets1.size(); ++i) {
            // Move all the red targets diagonally
            targets1[i]->moveX(1.5);
            targets1[i]->moveY(-1.5);

            // If a target has moved off the screen
            if (targets1[i]->getPosX() > width + targets1[i]->getSize().x / 2) {
                // Set it to the left of the screen so that it passes through again
                int targetOnRight = (targets1[i] == targets1[0]) ? targets1.size() - 1 : i - 1;
                targets1[i]->setPosX(10);
            }
            if (targets1[i]->getPosY() < -(targets1[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets1[i] == targets1[0]) ? targets1.size() - 1 : i - 1;
                targets1[i]->setPosY(590);
            }
        }

        for (int i = 0; i < targets2.size(); ++i) {
            // Move all the red targets diagonally
            targets2[i]->moveX(3);
            targets2[i]->moveY(-3);

            // If a target has moved off the screen
            if (targets2[i]->getPosX() > width + targets2[i]->getSize().x / 2) {
                // Set it to the left of the screen so that it passes through again
                int targetOnRight = (targets2[i] == targets2[0]) ? targets2.size() - 1 : i - 1;
                targets2[i]->setPosX(10);
            }
            if (targets2[i]->getPosY() < -(targets2[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets2[i] == targets2[0]) ? targets2.size() - 1 : i - 1;
                targets2[i]->setPosY(590);
            }
        }

        for (int i = 0; i < targets3.size(); ++i) {
            // Move all the purple targets diagonally
            targets3[i]->moveX(.5);
            targets3[i]->moveY(-.5);

            // If a target has moved off the screen
            if (targets3[i]->getPosX() > width + targets3[i]->getSize().x / 2) {
                // Set it to the left of the screen so that it passes through again
                int targetOnRight = (targets3[i] == targets3[0]) ? targets3.size() - 1 : i - 1;
                targets3[i]->setPosX(10);
            }
            if (targets3[i]->getPosY() < -(targets3[i]->getSize().y / 2)) {
                // Set it to the bottom of the screen so that it passes through again
                int targetBelow = (targets3[i] == targets3[0]) ? targets3.size() - 1 : i - 1;
                targets3[i]->setPosY(590);
            }
        }
    }
}

void Engine::render() {
    glClearColor(skyBlue.red,skyBlue.green, skyBlue.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            string message = "Press s to start!";
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            // Game instructions
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), 100, 1, vec3{1, 0, 0});
            string instMessage = "How to Play:";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 500, 1, vec3{1, 1, 1});
            instMessage = "Targets are flying around!";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 450, 1, vec3{1, 1, 1});
            instMessage = "When you click on a target, it";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 400, 1, vec3{1, 1, 1});
            instMessage = "will pop. Targets will move at";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 350, 1, vec3{1, 1, 1});
            instMessage = "different speeds and are";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 300, 1, vec3{1, 1, 1});
            instMessage = "different shapes and sizes.";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 250, 1, vec3{1, 1, 1});
            instMessage = "Try to be accurate!";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 200, 1, vec3{1, 1, 1});
            instMessage = "Good luck!";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 150, 1, vec3{1, 1, 1});
            break;
        }
        case level1: {
            grass1->setUniforms();
            grass1->draw();

            bottomBorder1->setUniforms();
            bottomBorder1->draw();

            topBorder1->setUniforms();
            topBorder1->draw();

            for (const auto& b  : targets3) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets2) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets1) {
                b->setUniforms();
                b->draw();
            }

            user->setUniforms();
            user->draw();

            bonusBox->setUniforms();
            bonusBox->draw();

            //bonusBox popup
            string bbMessage;
            bbMessage = "BONUS BOX!";
            while (bonusBox->getLeft() > 0 && bonusBox->getRight() < 800) {
                this->fontRenderer->renderText(bbMessage, width/2 - (12 * bbMessage.length()), 510, 1, vec3{1, 1, 1});
            }

            //Live score tracker
            stringstream ss;
            ss << score;
            string message = ss.str();
            ss << message;
            string ptTracker = "Score: " + message;
            this->fontRenderer->renderText(ptTracker, width/2 - (12 * ptTracker.length()), 540, 1, vec3{1, 1, 1});
            string hardM = "Mode: " + hard;
            this->fontRenderer->renderText(hardM, 30, 510, .5, vec3{1, 1, 1});
            break;
        }
        case level2: {
            grass2->setUniforms();
            grass2->draw();

            bottomBorder2->setUniforms();
            bottomBorder2->draw();

            topBorder2->setUniforms();
            topBorder2->draw();

            for (const auto& b  : targets3) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets2) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets1) {
                b->setUniforms();
                b->draw();
            }

            user->setUniforms();
            user->draw();

            bonusBox->setUniforms();
            bonusBox->draw();

            //bonusBox popup
            string bbMessage;
            bbMessage = "BONUS BOX!";
            while (bonusBox->getLeft() > 0 && bonusBox->getRight() < 800) {
                this->fontRenderer->renderText(bbMessage, width/2 - (12 * bbMessage.length()), 510, 1, vec3{1, 1, 1});
            }

            //Live score tracker
            stringstream ss;
            ss << score;
            string message = ss.str();
            ss << message;
            string ptTracker = "Score: " + message;
            this->fontRenderer->renderText(ptTracker, width/2 - (12 * ptTracker.length()), 540, 1, vec3{1, 1, 1});
            string hardM = "Mode: " + hard;
            this->fontRenderer->renderText(hardM, 30, 510, .5, vec3{1, 1, 1});
            break;
        }
        case level3: {
            grass3->setUniforms();
            grass3->draw();

            bottomBorder3->setUniforms();
            bottomBorder3->draw();

            topBorder3->setUniforms();
            topBorder3->draw();

            for (const auto& b  : targets3) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets2) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets1) {
                b->setUniforms();
                b->draw();
            }

            user->setUniforms();
            user->draw();

            bonusBox->setUniforms();
            bonusBox->draw();

            //bonusBox popup
            string bbMessage;
            bbMessage = "BONUS BOX!";
            while (bonusBox->getLeft() > 0 && bonusBox->getRight() < 800) {
                this->fontRenderer->renderText(bbMessage, width/2 - (12 * bbMessage.length()), 510, 1, vec3{1, 1, 1});
            }

            //Live score tracker
            stringstream ss;
            ss << score;
            string message = ss.str();
            ss << message;
            string ptTracker = "Score: " + message;
            this->fontRenderer->renderText(ptTracker, width/2 - (12 * ptTracker.length()), 540, 1, vec3{1, 1, 1});
            string hardM = "Mode: " + hard;
            this->fontRenderer->renderText(hardM, 30, 510, .5, vec3{1, 1, 1});
            break;
        }
        case level4: {
            grass4->setUniforms();
            grass4->draw();

            bottomBorder4->setUniforms();
            bottomBorder4->draw();

            topBorder4->setUniforms();
            topBorder4->draw();

            for (const auto& b  : targets3) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets2) {
                b->setUniforms();
                b->draw();
            }

            for (const auto& b : targets1) {
                b->setUniforms();
                b->draw();
            }

            user->setUniforms();
            user->draw();

            bonusBox->setUniforms();
            bonusBox->draw();

            //bonusBox popup
            string bbMessage;
            bbMessage = "BONUS BOX!";
            while (bonusBox->getLeft() > 0 && bonusBox->getRight() < 800) {
                this->fontRenderer->renderText(bbMessage, width/2 - (12 * bbMessage.length()), 510, 1, vec3{1, 1, 1});
            }

            //Live score tracker
            stringstream ss;
            ss << score;
            string message = ss.str();
            ss << message;
            string ptTracker = "Score: " + message;
            this->fontRenderer->renderText(ptTracker, width/2 - (12 * ptTracker.length()), 540, 1, vec3{1, 1, 1});
            string hardM = "Mode: " + hard;
            this->fontRenderer->renderText(hardM, 30, 510, .5, vec3{1, 1, 1});
            break;
        }
        case over: {
            int totalTime = endTime - startTime;
            stringstream ss;
            ss << totalTime;
            string message = "You win!";
            // Displays the message on the screen
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), 540, 1, vec3{1, 1, 0});
            string timeText = "It took you...";
            this->fontRenderer->renderText(timeText, width/2 - (12 * timeText.length()), 460, 1, vec3{1, 1, 1});
            string timeResult = " ";
            ss >> timeResult;
            this->fontRenderer->renderText(timeResult, width/2 - (12 * timeResult.length()), 407, 1, vec3{1, 1, 1});
            string timeQ = "seconds and";
            this->fontRenderer->renderText(timeQ, width/2 - (12 * timeQ.length()), 355, 1, vec3{1, 1, 1});
            stringstream ss2;
            ss2 << clicks;
            string clix = "";
            ss2 >> clix;
            this->fontRenderer->renderText(clix, width/2 - (12 * clix.length()), 300, 1, vec3{1, 1, 1});
            string postClix = "clicks to hit all targets!";
            this->fontRenderer->renderText(postClix, width/2 - (12 * postClix.length()), 260, 1, vec3{1, 1, 1});
            stringstream ss1;
            ss1 << accuracy;
            string aim = "";
            ss1 >> aim;
            string accResult = "Accuracy: " + aim + "%";
            this->fontRenderer->renderText(accResult, width/2 - (12 * accResult.length()), 220, 1, vec3{1, 1, 1});
            string restart = "Press 'r' to replay, or";
            this->fontRenderer->renderText(restart, width/2 - (12 * restart.length()), 150, 1, vec3{1, 1, 1});
            string levels = "press '1' '2' '3' or '4'";
            this->fontRenderer->renderText(levels, width/2 - (12 * levels.length()), 120, 1, vec3{1, 1, 1});
            string levels2 = "to jump to that level!";
            this->fontRenderer->renderText(levels2, width/2 - (12 * levels2.length()), 90, 1, vec3{1, 1, 1});
            string hardmode = "Press 'h' to enter hardmode!";
            this->fontRenderer->renderText(hardmode, width/2 - (12 * hardmode.length()), 60, 1, vec3{1, 1, 1});
            string hardmodeOff = "Press 'n' to exit hardmode!";
            this->fontRenderer->renderText(hardmodeOff, width/2 - (12 * hardmodeOff.length()), 30, 1, vec3{1, 1, 1});
            break;
        }
    }

    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}