#pragma once

#include <string>
#include <array>

#include <unordered_map>

#include "noise.h"
#include "thread.h"
#include "budget.h"

#include "C:\Users\morga\Documents\VS Includes\raylib-6.0_win64_msvc16\include\raylib.h"

//TODO: Move these constants to a config file or something. Clean up the code. Make it more readable. Add comments.

const float TILESIZE = 16.0f;
const int CHUNKSIZE = 16;

int RENDERDISTX = 16;
int RENDERDISTY = 9;

int CULLDISTX = RENDERDISTX * 2;
int CULLDISTY = RENDERDISTY * 2;

struct TileProperties {
    Color color = { 0x66, 0x38, 0x22, 0xFF };
    int id = 0;
};

enum class Tile : uint8_t {
    Snow,
    Stone,
    Grass,
    Dirt,
    Water,
    DeepWater,
    Null
};

TileProperties getTileProperties(Tile tile) {
    switch (tile) {
    case Tile::Dirt:      return { { 0x66, 0x38, 0x22, 0xFF }, 0 };
    case Tile::Snow:      return { { 0xEF, 0xEF, 0xEF, 0xFF }, 4 };
    case Tile::Stone:     return { { 0x67, 0x67, 0x67, 0xFF }, 3 };
    case Tile::Grass:     return { { 0x00, 0x67, 0x00, 0xFF }, 2 };
    case Tile::Water:     return { { 0x00, 0x0F, 0xFF, 0xFF }, 1 };
    case Tile::DeepWater: return { { 0x00, 0x0F, 0xAA, 0xFF }, 5 };
    case Tile::Null:      return { {}, -1 };
    default:              return { {}, -1 };
    }
}

Tile getTileFromElevation(long double e) {
    if (e < -0.1l) { return Tile::DeepWater; }
    else if (e < 0.0l) { return Tile::Water; }
    else if (e < 0.1l) { return Tile::Dirt; }
    else if (e < 0.6l) { return Tile::Grass; }
    else if (e < 0.8l) { return Tile::Stone; }
    else { return Tile::Snow; }
}

struct ChunkPos
{
    int x{};
    int y{};

    bool operator==(const ChunkPos& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct ChunkPosHash
{
    size_t operator()(const ChunkPos& p) const
    {
        size_t hx = std::hash<int>()(p.x);
        size_t hy = std::hash<int>()(p.y);
        return hx ^ (hy << 1);
    }
};

struct Chunk {
    std::array<Tile, CHUNKSIZE* CHUNKSIZE> rawdata{};
    ChunkPos pos;
    RenderTexture2D texture;
    bool dirty = true;
};

struct GeneratedChunkData {
    ChunkPos pos;
    std::array<Tile, CHUNKSIZE* CHUNKSIZE> rawdata;
};

class ChunkManager
{
private:
    Noise noise{};
    std::unordered_map<ChunkPos, Chunk, ChunkPosHash> chunks;

    // multi threading
    ThreadPool pool{ std::max(1u, std::thread::hardware_concurrency() - 2) };

    std::queue<GeneratedChunkData> completedChunks;
    std::mutex completedMutex;

    std::unordered_set<ChunkPos, ChunkPosHash> pendingChunks;
    std::mutex pendingMutex;

    // frame budget cursor
    std::unordered_map<ChunkPos, Chunk, ChunkPosHash>::iterator cleanCursor;
    bool cleanCursorValid = false;

    int updateCursorX = -RENDERDISTX;
    int updateCursorY = -RENDERDISTY;

public:

    Chunk* getChunk(ChunkPos pos)
    {
        auto it = chunks.find(pos);
        if (it == chunks.end())
            return nullptr;
        return &it->second;
    }

    std::mutex& getCompletedMutex() { return completedMutex; }
    std::mutex& getPendingMutex() { return pendingMutex; }

    std::unordered_set<ChunkPos, ChunkPosHash>& getPendingChunks() { return pendingChunks; }
    std::queue<GeneratedChunkData>& getCompletedChunks() { return completedChunks; }
    std::unordered_map<ChunkPos, Chunk, ChunkPosHash>& getChunks() { return chunks; }

    // Renders each tile as 1x1 pixel — GPU scales up at draw time via DrawTextureEx

    void BuildChunkTexture(Chunk& chunk)
    {
        if (!chunk.dirty) return;

        chunk.dirty = false;

        if (chunk.texture.id != 0)
            UnloadRenderTexture(chunk.texture);

        // 1 pixel per tile — 16x16 for CHUNKSIZE=16
        chunk.texture = LoadRenderTexture(CHUNKSIZE, CHUNKSIZE);

        SetTextureFilter(chunk.texture.texture, TEXTURE_FILTER_POINT);

        BeginTextureMode(chunk.texture);
        ClearBackground(BLANK);

        for (int y = 0; y < CHUNKSIZE; y++)
        {
            for (int x = 0; x < CHUNKSIZE; x++)
            {
                Tile tile = chunk.rawdata[y * CHUNKSIZE + x];
                TileProperties props = getTileProperties(tile);

                // 1x1 pixel per tile instead of TILESIZE x TILESIZE
                DrawRectangle(x, y, 1, 1, props.color);
            }
        }

        EndTextureMode();
    }

    void MakeChunk(ChunkPos& pos)
    {
        Chunk chunk;
        chunk.pos = pos;

        for (int y = 0; y < CHUNKSIZE; y++) {
            for (int x = 0; x < CHUNKSIZE; x++) {
                int globalX = pos.x * CHUNKSIZE + x;
                int globalY = pos.y * CHUNKSIZE + y;
                float elevation = noise.SampleElevation(globalX, globalY);
                chunk.rawdata[x + CHUNKSIZE * y] = getTileFromElevation(elevation);
            }
        }

        BuildChunkTexture(chunk);
        chunks.try_emplace(pos, std::move(chunk));
    }

    void clean(const Camera2D& camera, double msBudget = 0.5)
    {
        if (chunks.empty()) return;

        int rootX = (int)std::floor(camera.target.x / (TILESIZE * CHUNKSIZE));
        int rootY = (int)std::floor(camera.target.y / (TILESIZE * CHUNKSIZE));

        if (!cleanCursorValid || cleanCursor == chunks.end()) {
            cleanCursor = chunks.begin();
            cleanCursorValid = true;
        }

        FrameBudget budget(msBudget);

        while (budget.hasTime() && cleanCursor != chunks.end())
        {
            ChunkPos pos = cleanCursor->first;
            Chunk& chunk = cleanCursor->second;

            bool inRange =
                pos.x >= -CULLDISTX + rootX && pos.x <= CULLDISTX + rootX &&
                pos.y >= -CULLDISTY + rootY && pos.y <= CULLDISTY + rootY;

            if (!inRange)
            {
                UnloadRenderTexture(chunk.texture);
                cleanCursor = chunks.erase(cleanCursor);
                continue;
            }

            if (chunk.dirty)
                BuildChunkTexture(chunk);

            ++cleanCursor;
        }
        CleanPendingChunks(camera);
    }

    void DrawChunk(const Chunk& chunk, ChunkPos& pos)
    {
        Vector2 position = {
            std::round((float)pos.x * TILESIZE * CHUNKSIZE),
            std::round((float)pos.y * TILESIZE * CHUNKSIZE)
        };

        // Scale: each pixel in the 16x16 texture becomes TILESIZE pixels on screen
        // Flip Y via negative source height to correct RenderTexture orientation
        DrawTexturePro(
            chunk.texture.texture,
            { 0, 0, (float)CHUNKSIZE, -(float)CHUNKSIZE },         // src (flipped)
            { position.x, position.y, CHUNKSIZE * TILESIZE, CHUNKSIZE * TILESIZE }, // dst
            { 0, 0 },
            0.0f,
            WHITE
        );
    }

    void DrawGhostChunk(Chunk& chunk, ChunkPos& pos)
    {
        Vector2 position = {
            std::round((float)pos.x * TILESIZE * CHUNKSIZE),
            std::round((float)pos.y * TILESIZE * CHUNKSIZE)
        };

        DrawTexturePro(
            chunk.texture.texture,
            { 0, 0, (float)CHUNKSIZE, -(float)CHUNKSIZE },
            { position.x, position.y, CHUNKSIZE * TILESIZE, CHUNKSIZE * TILESIZE },
            { 0, 0 },
            0.0f,
            { 0x99, 0x99, 0x99, 0xFF }
        );
    }

    void submitGenerationJob(ChunkPos pos)
    {
        pool.enqueue([this, pos]() {

            // If this job was invalidated before it started,
            // don't generate anything.
            if (!IsPending(pos))
                return;

            GeneratedChunkData data;
            data.pos = pos;

            for (int y = 0; y < CHUNKSIZE; y++)
            {
                for (int x = 0; x < CHUNKSIZE; x++)
                {
                    // Allow cancellation during generation.
                    if (!IsPending(pos))
                        return;

                    int globalX = pos.x * CHUNKSIZE + x;
                    int globalY = pos.y * CHUNKSIZE + y;

                    float elevation =
                        noise.SampleElevation(
                            (float)globalX,
                            (float)globalY
                        );

                    data.rawdata[x + CHUNKSIZE * y] =
                        getTileFromElevation(elevation);
                }
            }

            // Camera may have moved while we were generating.
            if (!IsPending(pos))
                return;

            {
                std::lock_guard<std::mutex> lock(completedMutex);
                completedChunks.push(std::move(data));
            }
            });
    }

    void UpdateChunks(const Camera2D& camera, double msBudget = 0.5)
    {
        int rootX = (int)std::floor(camera.target.x / (TILESIZE * CHUNKSIZE));
        int rootY = (int)std::floor(camera.target.y / (TILESIZE * CHUNKSIZE));

        FrameBudget budget(msBudget);

        while (budget.hasTime())
        {
            if (updateCursorY > RENDERDISTY)
            {
                updateCursorX = -RENDERDISTX;
                updateCursorY = -RENDERDISTY;
                break;
            }

            ChunkPos pos = {
                updateCursorX + rootX,
                updateCursorY + rootY
            };

            if (!getChunk(pos))
            {
                std::lock_guard<std::mutex> lock(pendingMutex);

                if (!pendingChunks.count(pos))
                {
                    pendingChunks.insert(pos);
                    submitGenerationJob(pos);
                }
            }

            // ALWAYS advance the cursor
            updateCursorX++;

            if (updateCursorX > RENDERDISTX)
            {
                updateCursorX = -RENDERDISTX;
                updateCursorY++;
            }
        }
    }
    bool IsPending(ChunkPos pos)
    {
        std::lock_guard<std::mutex> lock(pendingMutex);
        return pendingChunks.count(pos) != 0;
    }
    bool IsInRange(ChunkPos pos, int rootX, int rootY, int distX, int distY) const
    {
        return
            pos.x >= rootX - distX &&
            pos.x <= rootX + distX &&
            pos.y >= rootY - distY &&
            pos.y <= rootY + distY;
    }
    void CleanPendingChunks(const Camera2D& camera)
    {
        int rootX = (int)std::floor(
            camera.target.x / (TILESIZE * CHUNKSIZE)
        );

        int rootY = (int)std::floor(
            camera.target.y / (TILESIZE * CHUNKSIZE)
        );

        std::lock_guard<std::mutex> lock(pendingMutex);

        for (auto it = pendingChunks.begin(); it != pendingChunks.end(); )
        {
            if (!IsInRange(
                *it,
                rootX,
                rootY,
                CULLDISTX,
                CULLDISTY))
            {
                it = pendingChunks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    void ProcessCompletedChunks(double msBudget = 2.0) {
        FrameBudget budget(msBudget);

        while (budget.hasTime()) {
            GeneratedChunkData data;

            {
                std::lock_guard<std::mutex> lock(completedMutex);
                if (completedChunks.empty()) return;
                data = std::move(completedChunks.front());
                completedChunks.pop();
            }

            Chunk chunk;
            chunk.pos = data.pos;
            chunk.rawdata = data.rawdata;
            chunk.dirty = true;

            BuildChunkTexture(chunk);

            chunks.try_emplace(data.pos, std::move(chunk));

            {
                std::lock_guard<std::mutex> lock(pendingMutex);
                pendingChunks.erase(data.pos);
            }
        }
    }

    void DrawChunks(const Camera2D& camera)
    {
        int rootX = (int)std::floor(camera.target.x / (TILESIZE * CHUNKSIZE));
        int rootY = (int)std::floor(camera.target.y / (TILESIZE * CHUNKSIZE));

        for (int y = -CULLDISTY; y <= CULLDISTY; y++) {
            for (int x = -CULLDISTX; x <= CULLDISTX; x++) {

                ChunkPos pos = { x + rootX, y + rootY };
                Chunk* c = getChunk(pos);

                if (pos.x >= -RENDERDISTX + rootX && pos.x <= RENDERDISTX + rootX &&
                    pos.y >= -RENDERDISTY + rootY && pos.y <= RENDERDISTY + rootY) {
                    if (c) DrawChunk(*c, pos);
                }
                else if (c) {
                    DrawGhostChunk(*c, pos);
                }
            }
        }
    }

    void DrawDiagnostics() {
        size_t pendingCount, completedCount;
        {
            std::lock_guard<std::mutex> lock(pendingMutex);
            pendingCount = pendingChunks.size();
        }
        {
            std::lock_guard<std::mutex> lock(completedMutex);
            completedCount = completedChunks.size();
        }

        DrawText(TextFormat("Pending gen: %zu", pendingCount), 10, 10, 18, GREEN);
        DrawText(TextFormat("Awaiting texture: %zu", completedCount), 10, 30, 18, GREEN);
        DrawText(TextFormat("Loaded chunks: %zu", chunks.size()), 10, 50, 18, GREEN);
    }
};

