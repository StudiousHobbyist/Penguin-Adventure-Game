#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include "world.h"

#include "C:\Users\morga\Documents\VS Includes\raylib-6.0_win64_msvc16\include\raylib.h"

// TODO: Move these constants to a config file or something. Clean up the code. Make it more readable. Add comments.
int ENTITYCAP = 2;


struct EntityPos {
    float x{};
    float y{};

    bool operator==(const EntityPos& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct EntityProperties {
    float speed = 20.0f;
};

class Entity {
private:
    EntityPos pos;
    EntityProperties props;

    float sizeX;
    float sizeY;
public:
    Entity(EntityPos pos, float sizeX, float sizeY, EntityProperties props) : pos(pos), sizeX(sizeX), sizeY(sizeY), props(props) {}

    EntityPos GetPos() {
        return pos;
    }

    ChunkPos GetChunkPos() const {
        float chunkPixels = TILESIZE * CHUNKSIZE;
        return ChunkPos{
            (int)std::floor(pos.x / chunkPixels),
            (int)std::floor(pos.y / chunkPixels)
        };
    }

    float wrap(float v, float d) const
    {

        v = std::fmod(v, d);
        if (v < 0) v += d;
        return v;
    }
    Tile GetTileAtWorldPosition(ChunkManager& cm, float x, float y)
    {
        float chunkPixels = CHUNKSIZE * TILESIZE;

        ChunkPos chunkPos{
            (int)std::floor(x / chunkPixels),
            (int)std::floor(y / chunkPixels)
        };

        Chunk* chunk = cm.getChunk(chunkPos);

        if (!chunk)
            return Tile::Null;

        float localX = x - chunkPos.x * chunkPixels;
        float localY = y - chunkPos.y * chunkPixels;

        int tileX = (int)std::floor(localX / TILESIZE);
        int tileY = (int)std::floor(localY / TILESIZE);

        if (tileX < 0 || tileX >= CHUNKSIZE ||
            tileY < 0 || tileY >= CHUNKSIZE)
            return Tile::Null;

        return chunk->rawdata[tileX + CHUNKSIZE * tileY];
    }

    std::vector<Tile> getTile(ChunkManager& cm)
    {
        float halfX = sizeX / 2.0f;
        float halfY = sizeY / 2.0f;

        return {
            GetTileAtWorldPosition(cm, pos.x + halfX, pos.y + halfY),
            GetTileAtWorldPosition(cm, pos.x - halfX, pos.y + halfY),
            GetTileAtWorldPosition(cm, pos.x + halfX, pos.y - halfY),
            GetTileAtWorldPosition(cm, pos.x - halfX, pos.y - halfY)
        };
    }

    void Draw(ChunkManager& cm) {
        ChunkPos chunkPos = GetChunkPos();

        std::string chunkPosStr = "(" + std::to_string(chunkPos.x) + ", " + std::to_string(chunkPos.y) + ")";

        std::string tileIDStr = "ID: ";

        for (Tile tile : getTile(cm))
        {
            tileIDStr += std::to_string(getTileProperties(tile).id) + ", ";
        }

        float fontSize = (sizeX + sizeY / 2);

        DrawText(chunkPosStr.c_str(), pos.x - sizeX / 2, pos.y - sizeY / 2 - fontSize, fontSize, BLACK);
        DrawText(tileIDStr.c_str(), pos.x - sizeX / 2, pos.y - sizeY / 2 - fontSize * 2, fontSize, BLACK);

        DrawRectangle(
            pos.x - sizeX / 2,
            pos.y - sizeY / 2,
            (int)sizeX,
            (int)sizeY,
            RED
        );
    }

    void Move(int x, int y, double dt) {
        pos.x += x * props.speed * dt;
        pos.y += y * props.speed * dt;
    }
};

class EntityManager {
private:
    std::vector<Entity> entities;
    int selectedEntityIndex = 0;

public:
    void AddEntity()
    {
        entities.emplace_back(EntityPos{ 0.0f , 0.0f }, 10.0f, 10.0f, EntityProperties{});
    }

    void DrawEntities(ChunkManager& cm, double dt) {
        for (auto& entity : entities)
        {
            entity.Draw(cm);

        }
    }

    Entity* GetEntity(int index) {
        if (index < 0 || index >= (int)entities.size()) return nullptr;
        return &entities[index];
    }

    Entity* CurrentEntity() {
        if (selectedEntityIndex < 0 || selectedEntityIndex >= (int)entities.size()) return nullptr;
        return &entities[selectedEntityIndex];
    }
};
