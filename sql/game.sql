------------------------------------------ CREATE TABLES ------------------------------------------

-- Global variables table
create table if not exists vars(
    rc          any,                        -- generic return code variable
    shouldSetup int not null,               -- flag which makes the application run setup code when true
    pWindow     int not null default(0),    -- pointer to GLFW window
    t           real,                       -- time when last frame started
    dt          real not null default(0.16),-- duration of last frame

    vertexShader int,
    fragmentShader int,
    shaderProgram int,
    vbo int,
    vao int,

    totalScore int not null default(0)

) strict;

-- We're doing ECS since it's basically a simplified version of the relational model
create table if not exists entities(
    id integer primary key,
    x real not null default(0),     -- position
    y real not null default(0),
    vx real not null default(0),    -- velocity
    vy real not null default(0),
    sx real not null default(0.1),  -- size
    sy real not null default(0.1),
    isPlayer int not null default(0),
    keepInBounds int not null default(0),
    deleteOutOfBounds int not null default(0),
    reloadLeft real not null default(0),
    reloadTime real not null default(0.3),
    isShooting int not null default(0),
    affiliation int,
    contactDamage real,
    health real,
    maxHealth real,
    iframes real not null default(0),
    age real not null default(0),
    maxAge real,
    hitCap int,
    scoreForKill int
) strict;

create table if not exists damageEvents(
    target_id integer,
    attacker_id integer,
    damage real
);

-- Rectangles drawn on screen
create table if not exists rects(
    x real not null,
    y real not null,
    sx real not null,
    sy real not null,
    r real not null,
    g real not null,
    b real not null,
    a real not null
) strict;

--------------------------------------------- SETUP -----------------------------------------------

-- Ensure there is a row in vars
insert into vars(shouldSetup)
select 1
where (select count(*) from vars) = 0;

-- Initialize GLFW and OpenGL
update vars
set rc = glfwInit()
where shouldSetup;

update vars
set pWindow = glfwCreateWindow(1280, 1280, "SQHell")
where shouldSetup;

update vars
set rc = glfwMakeContextCurrent(pWindow)
where shouldSetup;

update vars
set rc = gladLoadGL()
where shouldSetup;

select ImGuiCreateContext() 
from vars 
where shouldSetup;

select ImGui_ImplGlfw_InitForOpenGL(pWindow, 1)
from vars
where shouldSetup;

select ImGui_ImplOpenGL3_Init('#version 450')
from vars
where shouldSetup;

update vars
set vbo = glCreateBuffer(),
    vao = glCreateVertexArray(),
    vertexShader = glCreateShader(GL_VERTEX_SHADER()),
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER()),
    shaderProgram = glCreateProgram()
where shouldSetup;

select
    glShaderSource(vertexShader, readFileText("shaders/color.vert")),
    glShaderSource(fragmentShader, readFileText("shaders/color.frag"))
from vars
where shouldSetup;

select 
    glCompileShader(vertexShader), 
    glCompileShader(fragmentShader)
from vars where shouldSetup;

select 
    glAttachShader(shaderProgram, vertexShader), 
    glAttachShader(shaderProgram, fragmentShader)
from vars where shouldSetup;

select glLinkProgram(shaderProgram) 
from vars where shouldSetup;

select 
    glBindVertexArray(vao), 
    glBindBuffer(GL_ARRAY_BUFFER(), vbo) 
from vars where shouldSetup;

select 
    glEnableVertexAttribArray(0),
    glEnableVertexAttribArray(1),
    glVertexAttribPointer(0, 2, GL_FLOAT(), 0, 24, 0),
    glVertexAttribPointer(1, 4, GL_FLOAT(), 0, 24, 8)
from vars where shouldSetup;

-- Insert player entity
insert into entities(x,y,isPlayer,keepInBounds,health,maxHealth,affiliation)
select 0, 0, 1, 1, 100, 100, 0
from vars
where shouldSetup;

-- Insert an enemy
insert into entities(x,y,contactDamage,affiliation,health,maxHealth, scoreForKill)
select 0, 0.5, 10, 1, 100, 100, 100
from vars
where shouldSetup;


-- Complete setup
update vars
set shouldSetup = 0, t = glfwGetTime() - dt
where shouldSetup;

------------------------------------------- MAIN LOOP ---------------------------------------------

-- Exit condition

select exit(0)
from vars
where glfwWindowShouldClose(pWindow);

-- GUI BOILERPLATE

select ImGui_ImplOpenGL3_NewFrame();
select ImGui_ImplGlfw_NewFrame();
select ImGuiNewFrame();

-- GUI

select ImGuiBegin("Statistics");

    select ImGuiLabel("Score", totalScore)
    from vars;

    select ImGuiLabel("Total entities", count(*)) 
    from entities;

    select ImGuiLabel("Affiliation "||affiliation, count(*)) 
    from entities group by affiliation order by affiliation;

    select ImGuiLabel("With contact damage", count(*)) 
    from entities where contactDamage is not null;

    select ImGuiLabel("With health", count(*)) 
    from entities where health is not null;

    select ImGuiLabel("Player controlled", count(*)) 
    from entities where isPlayer;

select ImGuiEnd();

-- GAME UPDATE

-- Delta T management
update vars set dt = glfwGetTime() - t;
update vars set t = glfwGetTime();

-- Control player
update entities set vx=0, vy=0 where isPlayer;
update entities set vx=vx-1 where isPlayer and glfwGetKey((select pWindow from vars), "A") = GLFW_PRESS();
update entities set vx=vx+1 where isPlayer and glfwGetKey((select pWindow from vars), "D") = GLFW_PRESS();
update entities set vy=vy-1 where isPlayer and glfwGetKey((select pWindow from vars), "S") = GLFW_PRESS();
update entities set vy=vy+1 where isPlayer and glfwGetKey((select pWindow from vars), "W") = GLFW_PRESS();
update entities
set vx = cos(atan2(vy,vx)) * 0.5,
    vy = sin(atan2(vy,vx)) * 0.5
where isPlayer and (vx <> 0 or vy <> 0);

update entities 
set isShooting = (select glfwGetKey(pWindow, " ") = GLFW_PRESS() from vars)
where isPlayer;

-- Shooting
insert into entities(affiliation, contactDamage, deleteOutOfBounds, hitCap, x, y, sx, sy, vy)
select 
    affiliation, 10, 1, 1, x, y, 0.05, 0.05,
    case affiliation when 0 then 1 else -0.5 end
from entities
where isShooting and reloadLeft <= (select dt from vars);

update entities
set reloadLeft = reloadLeft + reloadTime
where isShooting and reloadLeft <= (select dt from vars);

-- Apply velocity, increase age, reload weapon
update entities
set x = x + vx * (select dt from vars),
    y = y + vy * (select dt from vars),
    age = age + (select dt from vars),
    iframes = max(0, iframes - (select dt from vars)),
    reloadLeft = max(0, reloadLeft - (select dt from vars));

-- Keep some entities in bounds
update entities
set x = max(sx/2-1, min(x, 1-sx/2)),
    y = max(sy/2-1, min(y, 1-sy/2))
where keepInBounds;

-- Projectile collision test
insert into damageEvents(target_id, attacker_id, damage)
select tgt.id, atk.id, atk.contactDamage
from entities tgt cross join entities atk
where tgt.health is not null
and tgt.iframes <= 0
and atk.contactDamage is not null
and atk.affiliation <> tgt.affiliation
and max(tgt.x-tgt.sx/2, atk.x-atk.sx/2) <= min(tgt.x+tgt.sx/2, atk.x+atk.sx/2)
and max(tgt.y-tgt.sy/2, atk.y-atk.sy/2) <= min(tgt.y+tgt.sy/2, atk.y+atk.sy/2);

update entities
set iframes = iframes + 0.25,
    health = max(0, health - (
        select max(damage)
        from damageEvents
        where target_id = id
    ))
where exists (select target_id from damageEvents where target_id = id);

update entities
set hitCap = max(0, hitCap-1)
where hitCap is not null 
and exists (select attacker_id from damageEvents where attacker_id = id);

delete from damageEvents;

-- Increase score for killed entities
update vars
set totalScore = totalScore + coalesce((
    select sum(scoreForKill)
    from entities
    where health = 0 and scoreForKill is not null
), 0);

-- Delete entities where applicable
delete from entities
where health <= 0
or age >= maxAge
or hitCap = 0;

delete from entities
where deleteOutOfBounds
and (
    x+sx/2 < -1 or
    x-sx/2 > 1 or
    y+sy/2 < -1 or
    y-sy/2 > 1
);

-- GAME RENDER

select glClearColor(
    (sin(t)+1)/2, 
    (sin(t+pi()*2/3)+1)/2, 
    (sin(t+pi()*4/3)+1)/2
) from vars;
select glClear(GL_COLOR_BUFFER_BIT());

select glUseProgram(shaderProgram) from vars;

-- Draw entities
insert into rects(x,y,sx,sy,r,g,b,a)
select x,y,sx,sy,1,1,1,1 from entities;

-- Draw health bars
insert into rects(x,y,sx,sy,r,g,b,a)
select x, y-0.7*sy, sx, 0.2*sy, 0,0,0,1 
from entities
where health is not null and maxHealth is not null;

insert into rects(x,y,sx,sy,r,g,b,a)
select x, y-0.7*sy, (sx-0.1*sy)*health/maxHealth, 0.1*sy, 1-health/maxHealth, health/maxHealth, 0, 1
from entities
where health is not null and maxHealth is not null;

select pushFloats(
    x-sx/2, y-sy/2, r,g,b,a,
    x+sx/2, y-sy/2, r,g,b,a,
    x-sx/2, y+sy/2, r,g,b,a,
    x+sx/2, y-sy/2, r,g,b,a,
    x-sx/2, y+sy/2, r,g,b,a,
    x+sx/2, y+sy/2, r,g,b,a
) from rects;

select glNamedBufferData(
    vbo, (select count(*)*6*24 from rects), 
    getFloats(), GL_STREAM_DRAW()
) from vars;

select clearFloats();

select glDrawArrays(GL_TRIANGLES(), 0, (select count(*)*6 from rects));
delete from rects;

select ImGuiRender();
select ImGui_ImplOpenGL3_RenderDrawData(ImGuiGetDrawData());

-- Polling etc

select glfwSwapBuffers(pWindow)
from vars;

select glfwPollEvents();


