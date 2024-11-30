#include "../../include/engine/Engine.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <format>
#include <string>

v3::v3(float x, float y, float z) : x(x), y(y), z(z) {};
v3::v3(float v) : x(v), y(v), z(v) {};
v3::v3(Vector3 v) : x(v.x), y(v.y), z(v.z) {};
v3::v3() : v3(0) {};

v3 v3::operator+(const v3& other) { return v3(x + other.x, y + other.y, z + other.z); }
v3 v3::operator-(const v3& other) { return v3(x - other.x, y - other.y, z - other.z); }
v3 v3::operator*(const v3& other) { return v3(x * other.x, y * other.y, z * other.z); }
v3 v3::operator/(const v3& other) { return v3(x / other.x, y / other.y, z / other.z); }

v3 v3::operator+=(const v3& other) { return *this = *this + other; }
v3 v3::operator-=(const v3& other) { return *this = *this - other; }
v3 v3::operator*=(const v3& other) { return *this = *this * other; }
v3 v3::operator/=(const v3& other) { return *this = *this / other; }

v3 v3::operator-(const float& other) { return v3(x - other, y - other, z - other); }
v3 v3::operator+(const float& other) { return v3(x + other, y + other, z + other); }
v3 v3::operator*(const float& other) { return v3(x * other, y * other, z * other); }
v3 v3::operator/(const float& other) { return v3(x / other, y / other, z / other); }
v3 v3::operator*=(const float& other) { return *this = *this * other; }
v3 v3::operator/=(const float& other) { return *this = *this / other; }

v3::operator Vector3() const { return {x, y, z}; }
v3::operator float*() const { return new float[3]{x, y, z}; }

std::string v3::to_string() const { return std::format("v3({:.1f}, {:.1f}, {:.1f})", x, y, z); }

int constrain(int val, int low, int high) { return std::min(std::max(val, low), high); }

void draw_text_codepoint_3d(int codepoint, Vector3 position, float font_size, Color color) {
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned
    // points to '?'
    Font font = GetFontDefault();
    int index = GetGlyphIndex(font, codepoint);
    float scale = font_size / (float)font.baseSize;
    bool backface = true;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x +=
        (float)(font.glyphs[index].offsetX - font.glyphPadding) / (float)font.baseSize * scale;
    position.z +=
        (float)(font.glyphs[index].offsetY - font.glyphPadding) / (float)font.baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for
    // outline/glow shader effects
    Rectangle srcRec = {font.recs[index].x - (float)font.glyphPadding,
                        font.recs[index].y - (float)font.glyphPadding,
                        font.recs[index].width + 2.0f * font.glyphPadding,
                        font.recs[index].height + 2.0f * font.glyphPadding};

    float width =
        (float)(font.recs[index].width + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;
    float height =
        (float)(font.recs[index].height + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;

    if (font.texture.id > 0) {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture
        // (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        rlCheckRenderBatchLimit(4 + 4 * backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f);  // Normal Pointing Up
        rlTexCoord2f(tx, ty);
        rlVertex3f(x, y, z);  // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th);
        rlVertex3f(x, y, z + height);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th);
        rlVertex3f(x + width, y,
                   z + height);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty);
        rlVertex3f(x + width, y, z);  // Top Right Of The Texture and Quad

        if (backface) {
            // Back Face
            rlNormal3f(0.0f, -1.0f, 0.0f);  // Normal Pointing Down
            rlTexCoord2f(tx, ty);
            rlVertex3f(x, y, z);  // Top Right Of The Texture and Quad
            rlTexCoord2f(tw, ty);
            rlVertex3f(x + width, y, z);  // Top Left Of The Texture and Quad
            rlTexCoord2f(tw, th);
            rlVertex3f(x + width, y,
                       z + height);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(tx, th);
            rlVertex3f(x, y,
                       z + height);  // Bottom Right Of The Texture and Quad
        }
        rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

void draw_text_3d(std::string text, v3 position, float font_size, Color color) {
    Font font = GetFontDefault();

    float text_offset_y = 0.0f;  // Offset between lines (on line break '\n')
    float text_offset_x = 0.0f;  // Offset X to next character to draw

    float scale = font_size / (float)font.baseSize;

    float line_spacing = 0.1f;

    for (size_t i = 0; i < text.length(); i++) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is
        // found (and return 0x3f) but we need to draw all of the bad bytes
        // using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n') {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            text_offset_y += scale + line_spacing / (float)font.baseSize * scale;
            text_offset_x = 0.0f;
        } else {
            if (codepoint != ' ' && codepoint != '\t') {
                draw_text_codepoint_3d(
                    codepoint,
                    v3(position.x + text_offset_x, position.y, position.z + text_offset_y),
                    font_size, color);
            }

            if (font.glyphs[index].advanceX == 0)
                text_offset_x +=
                    (float)(font.recs[index].width + line_spacing) / (float)font.baseSize * scale;
            else
                text_offset_x += (float)(font.glyphs[index].advanceX + line_spacing) /
                                 (float)font.baseSize * scale;
        }

        i += codepointByteCount;  // Move text bytes counter to next codepoint
    }
}
