#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#define PI 3.14159265
#include "Play.h"
#include <vector>
#include <cmath>

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;


enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_METEOR,
	TYPE_ASTEROID,
	TYPE_GEM,
	TYPE_PIECE,
	TYPE_DESTROYED,
	TYPE_FONT,
};

enum Agent8State
{
	STATE_FLY = 0,
	STATE_CRAWL,
	STATE_DEAD,
};

struct GameState
{
	int level_no = 1;
	int score = (level_no * 2) + 1;
	Agent8State agentState = STATE_FLY;
	int attachedAsteroidId;
	float x_pos_history[100] = { 0 };
	float y_pos_history[100] = { 0 };
	float angle_history[100] = { 0 };
};
GameState gameState;


void CreateAgent8();
void HandlePlayerControls();
void WhenLeavingDisplay(GameObject&);
void CreateMeteors();
void CreateAsteroids();
void UpdateMeteors();
void UpdateAsteroids();
void UpdateAgent8();
void UpdateGems();
void CreateAsteroidPieces();
void UpdateAsteroidPieces();
void DrawParticleTrails();


void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\spr_background.png");
	Play::PlayAudio("music");
	CreateAsteroids();
	CreateMeteors();
	CreateAgent8(); 
}

bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();
	UpdateMeteors();
	UpdateAsteroids();
	UpdateAgent8();
	UpdateGems();
	Play::DrawFontText("64px", "REMAINING GEMS: " + std::to_string(gameState.score),
		{ DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("132px", "LEVEL " + std::to_string(gameState.level_no),
		{ 50, DISPLAY_HEIGHT - 50 }, Play::LEFT);
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}
 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}




void CreateAgent8()
{
	Play::ColourSprite("particle", Play::cOrange);

	int agent8_id = Play::CreateGameObject(TYPE_AGENT8, 
		{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, 25, "spr_agent8_fly");
	GameObject& obj_agent8 = Play::GetGameObject(agent8_id);

	GameObject& obj_asteroid = Play::GetGameObjectByType(TYPE_ASTEROID);
	GameObject& obj_meteor = Play::GetGameObjectByType(TYPE_METEOR);

	if (Play::IsColliding(obj_agent8, obj_asteroid) || Play::IsColliding(obj_agent8, obj_meteor))
	{
		obj_agent8.pos.x -= 50;
	}
}

void HandlePlayerControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType( TYPE_AGENT8 );
	Play::SetSprite(obj_agent8, "spr_agent8_fly", 0);
	obj_agent8.scale = 0.7f;
	float a8_velocity = 2;

	if (Play::KeyDown(VK_RIGHT))
	{
		obj_agent8.rotation += 0.005;
		obj_agent8.velocity.x = a8_velocity * sin((PI/2) - obj_agent8.rotation);
		
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		obj_agent8.rotation -= 0.005;
		obj_agent8.velocity.y = a8_velocity * cos((PI/2) - obj_agent8.rotation);
	}
	else
	{
		obj_agent8.velocity.x = a8_velocity * sin((PI / 2) - obj_agent8.rotation);
		obj_agent8.velocity.y = a8_velocity * cos((PI / 2) - obj_agent8.rotation);
	}


	Play::UpdateGameObject(obj_agent8);
	WhenLeavingDisplay(obj_agent8);
	Play::DrawObjectRotated(obj_agent8);
}

void CreateMeteors()
{
	std::vector<int> vMeteors(gameState.level_no);
	for (int id : vMeteors)
	{
		int id = Play::CreateGameObject(TYPE_METEOR,
			{ 0, 0 },
			20, "spr_meteor_strip2");
		GameObject& obj_meteor = Play::GetGameObject(id);
		Play::SetSpriteOrigin("spr_meteor_strip2", 162, 64);
		obj_meteor.rotation = Play::RandomRollRange(175, 297) / 100;
		obj_meteor.animSpeed = 0.2f;
		obj_meteor.scale = 0.7f;
	}
}

void UpdateMeteors()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);

	for (int id : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id);

		obj_meteor.velocity.x = 5 * sin((PI / 2) - obj_meteor.rotation);
		obj_meteor.velocity.y = 5 * cos((PI / 2) - obj_meteor.rotation);

		if (gameState.agentState == STATE_FLY && Play::IsColliding(obj_meteor, obj_agent8))
		{
			Play::StopAudioLoop("music");
			Play::PlayAudio("combust");
			gameState.agentState = STATE_DEAD;
		}

		Play::UpdateGameObject(obj_meteor);
		WhenLeavingDisplay(obj_meteor);
		Play::DrawObjectRotated(obj_meteor);
	}

}

void CreateAsteroids()
{
	Play::ColourSprite("spr_gem", Play::Colour(Play::RandomRoll(100),
		Play::RandomRoll(100), Play::RandomRoll(100)));

	std::vector<int> vAsteroids ((gameState.level_no * 2) + 1);
	for (int id: vAsteroids)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID,
			{ Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_HEIGHT) },
			40, "spr_asteroid_strip2");
		GameObject& obj_asteroid = Play::GetGameObject(id);
		Play::SetSpriteOrigin("spr_asteroid_strip2", 105, 75);
		obj_asteroid.rotation = Play::RandomRoll(628) / 100;
		obj_asteroid.velocity.x = 3 * sin((PI / 2) - obj_asteroid.rotation);
		obj_asteroid.velocity.y = 3 * cos((PI / 2) - obj_asteroid.rotation);
		obj_asteroid.animSpeed = 0.2f;
		obj_asteroid.scale = 0.7f;
	}
}

void UpdateAsteroids()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id : vAsteroids)
	{
		GameObject& obj_asteroid = Play::GetGameObject(id);

		if (obj_asteroid.type == TYPE_ASTEROID &&  
			gameState.agentState == STATE_FLY && 
			Play::IsColliding(obj_asteroid, obj_agent8))
		{
			gameState.attachedAsteroidId = id;
			obj_agent8.rotation -= PI;
			gameState.agentState = STATE_CRAWL;			
		}
		
		if (gameState.agentState == STATE_CRAWL && Play::KeyPressed(VK_SPACE))
		{
			
			GameObject& obj_asteroid = Play::GetGameObject(gameState.attachedAsteroidId);
			obj_asteroid.type = TYPE_DESTROYED; 
			CreateAsteroidPieces();
			obj_agent8.rotSpeed = 0;
			Play::PlayAudio("explode");
			

			gameState.agentState = STATE_FLY;
			obj_agent8.pos.x = obj_agent8.pos.x + (60 * sin((PI / 2) - obj_agent8.rotation));
			obj_agent8.pos.y = obj_agent8.pos.y + (60 * cos((PI / 2) - obj_agent8.rotation));
			
			int id_gem = Play::CreateGameObject(TYPE_GEM, obj_asteroid.pos, 20, "gem");	
			GameObject& obj_gem = Play::GetGameObject(id_gem);
			if (!Play::IsVisible(obj_gem))
			{
				if (obj_gem.pos.x < 0)
					obj_gem.pos.x += 100;
				if (obj_gem.pos.x > DISPLAY_WIDTH)
					obj_gem.pos.x -= 100;
				if (obj_gem.pos.y < 0)
					obj_gem.pos.y += 100;
				if (obj_gem.pos.y > DISPLAY_HEIGHT)
					obj_gem.pos.y -= 100;
			}
		}

		Play::UpdateGameObject(obj_asteroid);
		WhenLeavingDisplay(obj_asteroid);
		Play::DrawObjectRotated(obj_asteroid);
	}

	UpdateAsteroidPieces();
}



void WhenLeavingDisplay(GameObject& obj)
{
	if (Play::IsLeavingDisplayArea(obj, Play::HORIZONTAL))
	{
		if (!Play::IsVisible(obj))
		{
			obj.pos.x = -(obj.pos.x) + DISPLAY_WIDTH;
		}
	}
	if (Play::IsLeavingDisplayArea(obj, Play::VERTICAL))
	{
		if (!Play::IsVisible(obj))
		{
			obj.pos.y = -(obj.pos.y) + DISPLAY_HEIGHT;
		}
	}
}

void UpdateGems()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);

	for (int id_gem : vGems)
	{
		GameObject& obj_gem = Play::GetGameObject(id_gem);
		bool hasCollided = false;

		if (gameState.agentState == STATE_FLY && Play::IsColliding(obj_gem, obj_agent8))
		{
			hasCollided = true;
			Play::PlayAudio("reward");
			gameState.score -= 1;	
		}
		Play::UpdateGameObject(obj_gem);
		Play::DrawObject(obj_gem);
		

		if (hasCollided == true)
		{
			Play::DestroyGameObject(id_gem);
		}
	}
}

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_asteroid = Play::GetGameObject(gameState.attachedAsteroidId);

	switch (gameState.agentState)
	{
	case STATE_FLY:
		HandlePlayerControls();		
		
		for (int i = 99; i > 0; i--)
		{
			//should probably use 2d vector for this
			gameState.x_pos_history[i] = gameState.x_pos_history[i - 1]; //move elements along 1
			gameState.y_pos_history[i] = gameState.y_pos_history[i - 1];
			gameState.angle_history[i] = gameState.angle_history[i - 1];
		}
		gameState.x_pos_history[0] = obj_agent8.pos.x;
		gameState.y_pos_history[0] = obj_agent8.pos.y;
		gameState.angle_history[0] = atan(obj_agent8.velocity.x / obj_agent8.velocity.y);

		DrawParticleTrails();

		if (gameState.score == 0)
		{
			Play::DestroyGameObjectsByType(TYPE_METEOR);
			gameState.level_no++;
			gameState.score = (gameState.level_no * 2) + 1;

			Play::DrawFontText("132px", "LEVEL " + std::to_string(gameState.level_no),
				{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);

			Play::PlayAudio("music");
			CreateAsteroids();
			CreateMeteors();
		}
		break;

	case STATE_CRAWL:
		Play::SetSprite(obj_agent8, "spr_agent8_left_strip7", 0);
		Play::SetSpriteOrigin("spr_agent8_left_strip7", -10, 64);
		Play::SetSpriteOrigin("spr_agent8_right_strip7", -10, 64);

		for (int i = 0; i < 100; i++)
		{
			gameState.x_pos_history[i] = 0;
			gameState.y_pos_history[i] = 0;
		}

		obj_agent8.pos = obj_asteroid.pos;

		if (Play::KeyDown(VK_LEFT))
		{
			obj_agent8.rotSpeed = -0.05f;
			Play::SetSprite(obj_agent8, "spr_agent8_left_strip7", 0.4f);
		}
		else if (Play::KeyDown(VK_RIGHT))
		{
			obj_agent8.rotSpeed = 0.05f;
			Play::SetSprite(obj_agent8, "spr_agent8_right_strip7", 0.4f);
		}
		else
		{
			obj_agent8.rotSpeed = 0;
		}

		

		break;

	case STATE_DEAD:
		Play::SetSprite(obj_agent8, "spr_agent8_dead_strip2", 0.2f);
		Play::SetSpriteOrigin(Play::GetSpriteId("spr_agent8_dead_strip2"), 94, 64);
		obj_agent8.velocity.x = 3 * sin((PI / 2) - obj_agent8.rotation);
		obj_agent8.velocity.y = 3 * cos((PI / 2) - obj_agent8.rotation);

		if (!Play::IsVisible(obj_agent8))
		{
			Play::DestroyGameObjectsByType(TYPE_METEOR);
			Play::DestroyGameObjectsByType(TYPE_ASTEROID);
			Play::DestroyGameObjectsByType(TYPE_GEM);

			for (int id_obj : Play::CollectAllGameObjectIDs())
				Play::GetGameObject(id_obj).type = TYPE_DESTROYED;


			gameState.level_no = 1;
			gameState.score = (gameState.level_no * 2) + 1;
			Play::PlayAudio("music");
			gameState.agentState = STATE_FLY;
			CreateAsteroids();
			CreateMeteors();
			CreateAgent8();
			
		}
		
		break;
	}

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea && gameState.agentState != STATE_DEAD)
	{
		WhenLeavingDisplay(obj_agent8);
	}

	Play::DrawObjectRotated(obj_agent8);
}

void CreateAsteroidPieces()
{
	GameObject& obj_destroy = Play::GetGameObject(gameState.attachedAsteroidId);
	for (int n = 0; n < 3; n++)
	{
		int id = Play::CreateGameObject(TYPE_PIECE, { obj_destroy.pos.x, obj_destroy.pos.y },
			0, "spr_asteroid_pieces_strip3");
		GameObject& obj_piece = Play::GetGameObject(id);
		obj_piece.rotation = (PI / 2) - (n * (2 * PI) / 3);
		obj_piece.frame = n;
		obj_piece.scale = 0.7f;
	}
}

void UpdateAsteroidPieces()
{
	std::vector<int> vPieces = Play::CollectGameObjectIDsByType(TYPE_PIECE);
	for (int id : vPieces)
	{
		GameObject& obj_piece = Play::GetGameObject(id);
		Play::SetGameObjectDirection(obj_piece, 5, obj_piece.rotation);
		
		Play::UpdateGameObject(obj_piece);
		Play::DrawObject(obj_piece);

		if (!Play::IsVisible(obj_piece))
		{
			Play::DestroyGameObject(id);
		}
	}
}

void DrawParticleTrails()
{
	int spriteId = Play::GetSpriteId("particle");
	float transparency = 1.;

	for (int i = 4; i < 100; i *= 1.3)
	{
		int temp = Play::RandomRollRange(-1, 1);
		float theta = temp * (PI / 2) + gameState.angle_history[i];
		float pos_x = gameState.x_pos_history[i] + 4*cos(theta);
		float pos_y = gameState.y_pos_history[i] + 4*sin(theta);
		Play::DrawSpriteTransparent(spriteId, { pos_x, pos_y }, 0, transparency);
		transparency *= 0.9;
	}
}