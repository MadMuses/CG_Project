#include "main.h"

int main(void)
{
    // Initalise window and OpenGl functions
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(windowWidth, windowHeight, "Final project", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

    // Prepare shadow map size for shadow mapping. Usually this is the size of the window itself, but on some platforms like Mac this can be 2x the size of the window. Use glfwGetFramebufferSize to get the shadow map size properly.
    glfwGetFramebufferSize(window, &depthMapWidth, &depthMapHeight);

    // Background
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Create a frame buffer
    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

    // Create Framebuffer Texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depthMapWidth, depthMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete." << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Load shaders
    shaders = LoadShaders();

    // Generate positions :

    // Doing grass

    // Creating the grid with all positions
    std::vector<glm::vec3> pos_vector = calcDomeGrid(7,0,0);

    //---- Based on : https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
    // Adapted to glm::vec3 using ChatGPT

    // Remove doubles
    // Creating a comparator
    auto vec3_less = [](const glm::vec3& a, const glm::vec3& b) {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    };

    // Sort vector
    std::sort(pos_vector.begin(), pos_vector.end(), vec3_less);

    // Use std::unique to find doubles
    auto it = std::unique(pos_vector.begin(), pos_vector.end(), [](const glm::vec3& a, const glm::vec3& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    });

    // Remove the doubles
    pos_vector.erase(it, pos_vector.end());

    //----

    // Shuffle the vector
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pos_vector.begin(), pos_vector.end(),g);

    // Put the positions from pos_vector in the grid_pos
    size_t index = 0;
    GLfloat grid_pos[pos_vector.size() * 3];
    for (const auto& vec : pos_vector) {
        grid_pos[index++] = vec.x;
        grid_pos[index++] = vec.y;
        grid_pos[index++] = vec.z;
    };
    int grassAmount = pos_vector.size()/4;

    // Create the grass position attributes
    GLfloat grass_pos_2[grassAmount*3];
    GLfloat grass_pos_3[grassAmount*3];
    GLfloat grass_pos_41[grassAmount*3];
    GLfloat grass_pos_42[grassAmount*3];

    // Get values from grid_pos
    std::memcpy(grass_pos_2,    grid_pos + 0,                   grassAmount * 3 * sizeof(GLfloat));
    std::memcpy(grass_pos_3,    grid_pos + grassAmount*3,   grassAmount * 3 * sizeof(GLfloat));
    std::memcpy(grass_pos_41,   grid_pos + grassAmount*6,   grassAmount * 3 * sizeof(GLfloat));
    std::memcpy(grass_pos_42,   grid_pos + grassAmount*9,  grassAmount * 3 * sizeof(GLfloat));

    // Create scale and rotation
    GLfloat grass_scl[grassAmount];
    GLfloat grass_angl[grassAmount];

    for (int i=0; i < grassAmount; i++)
    {
        grass_scl[i] = (95 + rand() % 10)/100.0f;
        grass_angl[i] = (-600 + rand() % 1200)/10.0f;
    }

    // FLOWER PATCH LETS GOO
    int flowerAmount = 52;
    GLfloat flowers_scl[flowerAmount];
    GLfloat flowers_angl[flowerAmount];
    GLfloat flowers_pos[flowerAmount*3];

    int z = 9;
    int fIndex = 0;
    for (int i=0; i < 4; ++i)
    {
        if (i >=2){z-=2;}
        for (int j=0; j < z; ++j)
        {
            flowers_pos[fIndex++] = i*7;
            flowers_pos[fIndex++] = 0;
            flowers_pos[fIndex++] = j*7 - (z-1)*3.5;

            if (i > 0)
            {
                flowers_pos[fIndex++] = -i*7;
                flowers_pos[fIndex++] = 0;
                flowers_pos[fIndex++] = j*7 - (z-1)*3.5;
            }
        }
    }

    for (int i = 0; i < flowerAmount; ++i)
    {
        flowers_scl[i] = (95 + rand() % 30)/100.0f;
        flowers_angl[i] = (-600 + rand() % 1200)/10.0f;
    }

    // Planting the trees
    GLfloat oak_pos[9] = {
        -70.0f,0.0f,-28.0f,
        63.0f,0.0f,35.0f,
        35.0f,0.0f,-42.0f
    };
    GLfloat oak_scl[3] = {0.5f,1.1f,0.9f};
    GLfloat oak_angl[3] = {0.0f,63.0f,45.0f};

    GLfloat spruce_pos[6] = {
        -14.0f,0.0f,21.0f,
        -63.0f,0.0f,49.0f
    };
    GLfloat spruce_scl[2] = {0.9f,1.4f};
    GLfloat spruce_angl[2] = {0.0f,63.0f};

// Create all of the objects :

    // Basic objects
    Skybox skybox;

    // Render ships
    gltfObj virgo,gemini,scorpio,virgo1,gemini1,scorpio1;
    gltfObj ships[6] = {virgo,gemini,scorpio,virgo1,gemini1,scorpio1};

    // Most complicated to setup (instancing) : Plants
    gltfObj grass2,grass3,grass41,grass42;
    gltfObj grass[4] = {grass2,grass3,grass41,grass42};

    // Placing the flowers
    gltfObj flowers;
    gltfObj flowers2;

    // Static Obj : Trees
    gltfObj spruce,oak;

    // Static object : The dome
    gltfObj dome;

    // Static object : Doors
    gltfObj door;

// Initialise objects :

    // initializing objects
    skybox.initialize(glm::vec3(boundary*0.6));

    // Static Obj : Grass blocks
    // Grass elements
    std::string names[4] = {"grass_2","grass_3","grass_4.1","grass_4.2"};

    // Position + shadow inits
    for (int i = 0; i < 4; ++i)
    {
        grass[i].init_s();
        grass[i].init_plmt(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.5f * worldScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    }

    // Instancing position buffer changes so we init them here
    grass[0].init_i(grassAmount,grass_pos_2,grass_scl,grass_angl);
    grass[1].init_i(grassAmount,grass_pos_3,grass_scl,grass_angl);
    grass[2].init_i(grassAmount,grass_pos_41,grass_scl,grass_angl);
    grass[3].init_i(grassAmount,grass_pos_42,grass_scl,grass_angl);

    // Global init
    for (int i = 0; i < 4; ++i)
    {
        std::string modelPath = "../assets/models/nature/" + names[i] + ".gltf";
        grass[i].init(shaders["obj_si"],shaders["obj_dpth_i"],i,modelPath.c_str(), NULL);
    }

    oak.init_s();
    oak.init_plmt(glm::vec3(0.0f),glm::vec3(worldScale*2.5),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    oak.init_i(3,oak_pos,oak_scl,oak_angl);
    oak.init(shaders["obj_si"],shaders["obj_dpth_i"],5,"../assets/models/nature/oak.gltf", "../assets/textures/nature/trees.png");

    spruce.init_s();
    spruce.init_plmt(glm::vec3(0.0f),glm::vec3(worldScale*2.5),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    spruce.init_i(2,spruce_pos,spruce_scl,spruce_angl);
    spruce.init(shaders["obj_si"],shaders["obj_dpth_i"],6,"../assets/models/nature/spruce.gltf", "../assets/textures/nature/trees.png");

    flowers.init_plmt(glm::vec3(-7.0f * 7.0f, 0.0f, -9.0f * 7.0f),glm::vec3(0.5f * worldScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    flowers.init_s();
    flowers.init_i(flowerAmount,flowers_pos,flowers_scl,flowers_angl);
    flowers.init(shaders["obj_si"],shaders["obj_dpth_i"],7,"../assets/models/nature/flower.gltf", "../assets/textures/nature/flowers.png");

    flowers2.init_plmt(glm::vec3(2.0f * 7.0f, 0.0f, 9.0f*7.0f),glm::vec3(0.5f * worldScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    flowers2.init_s();
    flowers2.init_i(flowerAmount,flowers_pos,flowers_scl,flowers_angl);
    flowers2.init(shaders["obj_si"],shaders["obj_dpth_i"],8,"../assets/models/nature/flower.gltf", "../assets/textures/nature/flowers.png");

    // Dome
    dome.init_s();
    dome.init_plmt(glm::vec3(0.0f),glm::vec3(domeScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    dome.init(shaders["obj_s"],shaders["obj_dpth"],9,"../assets/models/dome/dome.gltf", NULL);

    // Doors
    door.init_plmt(glm::vec3(182.0f,0.0f,-12.5f),glm::vec3(25.0f,25.0f,50.0f),glm::vec3(0.0f,1.0f,0.0f),180.0f);
    door.init(shaders["obj_nl"],shaders["obj_dpth"],20,"../assets/models/dome/door.gltf", "../assets/textures/dome/door.png");

    // Ships
    prepShips(shaders,ships,10);

    gltfObj flame;
    flame.init_a();
    flame.init_plmt(glm::vec3(0.0f,-7.0f,228.0f),glm::vec3(3.5*worldScale),glm::vec3(0.0f,0.0f,1.0f),90.0f);
    flame.init(shaders["obj_def"],shaders["obj_dpth"],17, "../assets/models/dome/flame.gltf", "../assets/textures/dome/flame.png");

    gltfObj flame2;
    flame2.init_a();
    flame2.init_plmt(glm::vec3(0.0f,-7.0f,-220.0f),glm::vec3(3.5*worldScale),glm::vec3(0.0f,0.0f,1.0f),90.0f);
    flame2.init(shaders["obj_def"],shaders["obj_dpth"],18, "../assets/models/dome/flame.gltf", "../assets/textures/dome/flame.png");

    // Create robot
    gltfObj robot;
    robot.init_a();
    robot.init_plmt(glm::vec3(126.0f,3.0f,-31.5f),glm::vec3(worldScale),glm::vec3(0.0f,1.0f,0.0f),-60.0f);
    robot.init(shaders["obj_s"],shaders["obj_dpth"],19,"../assets/models/bot/botorobot.gltf", NULL);


// The two different cameras

    // Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

    // Light POV Camera setup
    glm::mat4 lightViewMatrix, lightProjectionMatrix;
    lightProjectionMatrix = glm::perspective(glm::radians(depthFoV), (float)depthMapWidth / depthMapHeight, depthNear, depthFar);

// "Game" loop
    do
    {
    // Managing the depth texture creation
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        lightViewMatrix = glm::lookAt(lightPosition,depthlookat,lightUp);
        glm::mat4 lvp = lightProjectionMatrix * lightViewMatrix;

        dome.depthRender(lvp);
        robot.depthRender(lvp);

        flowers.depthRender(lvp);
        flowers2.depthRender(lvp);

        for (int i =0; i < 4; i++)
        {
            grass[i].depthRender(lvp);
        }

        oak.depthRender(lvp);
        spruce.depthRender(lvp);

        if (saveDepth) {
            std::string filename = "depth_camera.png";
            saveDepthTexture(depthFBO, filename);
            std::cout << "Depth texture saved to " << filename << std::endl;
            saveDepth = false;
        }

    // Rendering the scene
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update states for animation
        double currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        // Rendering
        viewMatrix = glm::lookAt(eye_center, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;

        // Change the mod values if we are far in space
        dome.init_plmt_mod(domeSclMod, domeSclMod);
        door.init_plmt_mod(domeSclMod, domeSclMod);

        flowers.init_plmt_mod(domeSclMod, domeSclMod);
        flowers2.init_plmt_mod(domeSclMod, domeSclMod);
        for (int i =0; i < 4; i++){grass[i].init_plmt_mod(domeSclMod, domeSclMod);}
        oak.init_plmt_mod(domeSclMod, domeSclMod);
        spruce.init_plmt_mod(domeSclMod, domeSclMod);
        flame.init_plmt_mod(domeSclMod, domeSclMod);
        flame2.init_plmt_mod(domeSclMod, domeSclMod);
        robot.init_plmt_mod(domeSclMod, domeSclMod);

        // Classic render
        dome.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        // We stop rendering the dome interior when the observer is far enough away to save performances as they cannot be seen anymore
        if (domeSclMod >= 0.8f)
        {
            flowers.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
            flowers2.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
            for (int i =0; i < 4; i++){grass[i].render(vp,lightPosition,lightIntensity,lvp,depthTexture);}
        }
        if (domeSclMod >= 0.6f)
        {
            oak.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
            spruce.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
        }

        // Render the flame
        flame.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
        flame2.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        // Render the bot
        robot.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        // Placing the skybox
        skybox.position = skyboxPosOffset; // New pos = offset because skybox is initialized at (0,0,0)
        skybox.render(vp, glm::vec3(skybox.scale*skyboxSclMod));

        // Render and move ships
        moveShips(ships, 0.5f);
        for (int i =0; i < 6; i++)
        {
            ships[i].render(vp,lightPosition,lightIntensity,lvp,depthTexture);
        }

        // Handling door opening/closing
        if (glm::length(eye_center - glm::vec3(160.0f,20.0f,0.0f)) < 70.0f&& door.position.y > -30.0f)
        {
            door.position.y -= 0.1f;
        }else
        {
            if (door.position.y < 0.0f)
            {
                door.position.y += 0.1f;
            }
        }

        // Render the door
        door.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        // Count number of frames over a few seconds and take average
        calcframerate();

        if (playAnimation) {
            thetime += deltaTime * playbackSpeed;
            flame.update(thetime);
            flame2.update(thetime);
            robot.update(thetime);
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (!glfwWindowShouldClose(window));

    // Clean up
    skybox.cleanup();
    dome.cleanup();
    flame.cleanup();
    flame2.cleanup();
    door.cleanup();
    flowers.cleanup();
    robot.cleanup();
    oak.cleanup();
    spruce.cleanup();

    for (int i=0; i < 6;i++){ships[i].cleanup();}
    for (int i=0; i < 4;i++){grass[i].cleanup();}

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
};