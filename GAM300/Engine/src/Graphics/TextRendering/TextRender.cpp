#include "TextRender.h"

#include <fstream>
#include "System/Scene/SceneSystem.h"
#include "Graphics/RenderManager/RenderManager.h"
#include "GL/glew.h"

#include "ft2build.h"
#include FT_FREETYPE_H

const std::array<std::string, (unsigned)TranslationManager::Languages::TOTAL> TranslationManager::LanguageNames =
    { "English", "Spanish" };

const std::array<std::string, (unsigned)TextComponent::Alignment::TOTAL> TextComponent::mAlignmentNames =
    { "Left", "Centered" };

GFX::Font::~Font()
{
    // Release all the textures and clear the character map
    ClearData();
}

GFX::Font::Character LoadCharacter(unsigned c, FT_Face& face)
{
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        return { (GLuint)-1, {}, {}, 0 };
    }

    // Save the generated info into a texture (and then that texture to a character)
    GFX::Font::Character curr_char{};

    glGenTextures(1, &curr_char.mTextureID);
    glBindTexture(GL_TEXTURE_2D, curr_char.mTextureID);
    glTexImage2D(GL_TEXTURE_2D,
        0, GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0, GL_RED,
        GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set the rest of the options
    curr_char.mSize = { face->glyph->bitmap.width, face->glyph->bitmap.rows };
    curr_char.mBearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
    curr_char.mAdvance = face->glyph->advance.x;

    // Add the character to the map, ready for whenever we need to draw it
    return curr_char;
}

void GFX::Font::LoadFont(std::string const& path)
{
    mProperlyLoaded = true;
    ClearData();
    GenerateQuad();

    // Initialize the FreeType library
    FT_Library mFTLibrary;
    if (FT_Init_FreeType(&mFTLibrary))
    {
        std::cerr << "Could not init FreeType Library" << std::endl;
        return;
    }

    // We could proably also store the face, but is probably not needed since characters
    // ar going to be all generated at this function call
    FT_Face face;
    if (FT_New_Face(mFTLibrary, path.data(), 0, &face))
    {
        return;
    }

    // Set the font size (we set the hight and the width is computed based on it)
    FT_Set_Pixel_Sizes(face, 0, height);

    // We will only be using one channel so the data may be unaligned
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);     
    
    // Load the texture of each character and save its properties
    for (unsigned c = 0; c < 256; c++) {
        auto ch = LoadCharacter(c, face);
        if (ch.mTextureID != -1)
            mCharacters[c] = ch;
    }

    // Restore the initial value
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // Clean up
    FT_Done_Face(face);
    FT_Done_FreeType(mFTLibrary);
}

void GFX::Font::GenerateQuad()
{
    // Generate the buffers
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO);

    // Bind everything
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    // Set the data (position and uvs)
    struct TextVtx { float pos[2]; float uvs[2]; } vertices[4] = 
    {
         0.5,  0.5,         1, 1,       // Top Right
        -0.5,  0.5,         0, 1,       // Top Left
        -0.5, -0.5,         0, 0,       // Bot Left
         0.5, -0.5,         1, 0        // Bot Right
    };
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(TextVtx), vertices, GL_STATIC_DRAW);

    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVtx), (void*)0);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVtx),
        (void*)offsetof(TextVtx, uvs));

    // Element data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    constexpr std::array<unsigned, 6> mQuadIndices = { 0,1,2,0,2,3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mQuadIndices.size() * sizeof(unsigned int),
        mQuadIndices.data(), GL_STATIC_DRAW);

    // Clear
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GFX::Font::DeleteQuad()
{
    // Delete the OpenGL resources used
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mEBO);
}

void GFX::Font::ClearData()
{
    for (auto& c : mCharacters) {
        glDeleteTextures(1, &c.second.mTextureID);
    }
    mCharacters.clear();

    DeleteQuad();
}



void TranslationManager::ChangeLanguage(Languages _lang)
{ 
    if (_lang != Languages::TOTAL) 
        mCurrLanguage = _lang;
}

void TranslationManager::ChangeLanguage(std::string& _lang)
{
    // Check if the given language is one we have stored
    unsigned i = 0;
    for (auto& l : LanguageNames) {
        if (l == _lang) {
            mCurrLanguage = (Languages)i;
            return;
        }
    }

    // Default case, none of the strings were what we were looking for
    std::cerr << "Could not find the specified language : " << _lang << std::endl;
    return;
}

std::string TranslationManager::GetMessage(unsigned id) const
{
    // Check if the language is valid
    if ((unsigned)mCurrLanguage >= (unsigned)Languages::TOTAL) {
        std::cerr << "Current Language outside the valid range" << std::endl;
        return std::string();
    }

    // Find the right mesage
    auto found = mMessages.find(id);
    if (found != mMessages.end()) {
        return found->second.mVersions[(unsigned)mCurrLanguage];
    }

    // Default return, no string
    return std::string();
}

#ifdef EDITOR
void TranslationManager::Edit()
{
    ImGui::Begin("AllMessages");

    if (ImGui::TreeNode("Messages")) {

        // Display all messages with their english version
        ImGui::Text("Maximum message size is 100");
        for (auto& m : mMessages) {
            ImGui::PushID(m.first);
            if (ImGui::TreeNodeEx(std::string("Message: " + std::to_string(m.first)).c_str())) {

                // Do the same thing for each language
                unsigned lang = 0;
                for (auto& s : m.second.mVersions) {
                    char temp[100];
                    strcpy(temp, s.c_str());

                    ImGui::PushItemWidth(160);
                    ImGui::InputText(LanguageNames[lang++].c_str(), temp, 100);

                    s = temp;
                }

                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    if (ImGui::Button("NewText"))
        mMessages[++mHighestID] = {};

    ImGui::End();
}
void TranslationManager::PickText(unsigned* _out) const {

    // Pick the actual message to be displayed
    auto& AllMessages = TranslationMgr.GetAllMessages();
    if (AllMessages.size() == 0) {
        ImGui::Text("No messages to pick from");
    }
    else {
        if (AllMessages.find(*_out) == AllMessages.end())
            *_out = AllMessages.begin()->first;

        if (ImGui::BeginCombo("Message", AllMessages.find(*_out)->second.mVersions[0].data()))
        {
            for (auto& m : AllMessages)
            {
                if (ImGui::Selectable(m.second.mVersions[0].data()))
                    *_out = m.first;
            }
            ImGui::EndCombo();
        }
    }
}
#endif

void TranslationManager::ToJson() const
{
    json j;

    std::ofstream outFile(mSavePath.c_str());
    if (outFile.good() && outFile.is_open()) {
        for (auto& m : mMessages) {

            json k;

            k["ID"] << m.first;
            for (unsigned i = 0; i < (unsigned)Languages::TOTAL; ++i) {
                k[LanguageNames[i]] << m.second.mVersions[i];
            }

            j.push_back(k);
        }

        outFile << std::setw(4) << j;
        outFile.close();
    }
}

void TranslationManager::FromJson()
{
    std::ifstream inFile(mSavePath.c_str());
    assert(inFile.good());

    json j;

    unsigned highest = 0;
    if (inFile.good() && inFile.is_open() && inFile.peek() != std::ifstream::traits_type::eof()) {
        inFile >> j;

        for (auto& k : j) {

            unsigned id = 0;
            k["ID"] >> id;
            mHighestID = glm::max(mHighestID, id);

            mMessages[id] = {};
            auto& curr_msg = mMessages[id];

            for (unsigned i = 0; i < (unsigned)Languages::TOTAL; ++i) {
                k[LanguageNames[i]] >> curr_msg.mVersions[i];
            }
        }

        inFile.close();
    }
}






void TextComponent::Render()
{
    if (!mbVisible) return;

    std::string msg = use_internal_message ? current_message 
        : TranslationMgr.GetMessage(mMessageID);

    RenderMgr.RenderText(mFont, mScale, mOffset, mColor, mOwner->mTransform, mbHorizontalBill, 
        mbVerticalBill, mAlignment, msg);
}

void TextComponent::SetText(std::string msg)
{ 
    current_message = msg; 
    use_internal_message = true;
}

IComp* TextComponent::Clone()
{
    return Scene.CreateComp<TextComponent>(mOwner->GetSpace(), this);
}

#ifdef EDITOR
bool TextComponent::Edit()
{
    if (ImGui::TreeNode("Font")) {
        auto& fonts = ResourceMgr.GetResourcesOfType<GFX::Font>();
        if (ImGui::BeginCombo("Choose Font", mFont->getName().c_str()))
        {
            for (auto& font : fonts)
            {
                if (ImGui::Selectable(font.first.data()))
                {
                    mFont = std::reinterpret_pointer_cast<TResource<GFX::Font>>(font.second);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Text")) {

        // Message
        TranslationMgr.PickText(&mMessageID);

        // Color
        ImGui::ColorEdit4("Color", &mColor.r);
        IRenderable::Edit();

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("TextPosition")) {

        // Alignment
        if (mAlignment >= Alignment::TOTAL) { mAlignment = Alignment::Left; }
        if (ImGui::BeginCombo("Alignment", mAlignmentNames[(unsigned)mAlignment].c_str()))
        {
            for (unsigned i = 0; i < (unsigned)Alignment::TOTAL; ++i)
            {
                if (ImGui::Selectable(mAlignmentNames[(unsigned)i].c_str()))
                    mAlignment = (Alignment)i;
            }
            ImGui::EndCombo();
        }

        // Position and scale
        ImGui::DragFloat3("Offset", &mOffset.x);
        ImGui::DragFloat2("Scale",  &mScale.x, 0.05f);

        // Billboarding
        ImGui::Checkbox("HorizontalBillboarding",   &mbHorizontalBill);
        ImGui::Checkbox("VerticalBillboarding",     &mbVerticalBill);

        ImGui::TreePop();
    }

    return false;
}
#endif

void TextComponent::ToJson(nlohmann::json& j) const
{
    j["mFont"]      << mFont->getName();
    j["mMessageID"] << mMessageID;
    j["mAlignment"] << (unsigned)mAlignment;
    j["mOffset"]    << mOffset;
    j["mColor"]     << mColor;
    j["mScale"]     << mScale;

    IRenderable::ToJson(j);

    j["mbHorizontalBill"]   << mbHorizontalBill;
    j["mbVerticalBill"]     << mbVerticalBill;
}

void TextComponent::FromJson(nlohmann::json& j)
{
    // Get the font type
    std::string mFontPath; 
j["mFont"] >> mFontPath;
    mFont = ResourceMgr.GetResource<GFX::Font>(mFontPath.data());
    if (!mFont || !mFont->get()) {
        std::cerr << "Could not load font : " << mFontPath << std::endl;
    }

    // Serialize one message/text/string. I suggest that if we want multiple strings, 
    // create a component for that
    j["mMessageID"] >> mMessageID;

    // Other variables
    j["mOffset"]    >> mOffset;
    j["mColor"]     >> mColor;
    j["mScale"]     >> mScale;
    IRenderable::FromJson(j);

    unsigned al = 0;
    j["mAlignment"] >> al;
    mAlignment = (Alignment)al;

    // Billboarding
    if (j.find("mbHorizontalBill") != j.end()) {
        j["mbHorizontalBill"]   >> mbHorizontalBill;
        j["mbVerticalBill"]     >> mbVerticalBill;
    }
}
