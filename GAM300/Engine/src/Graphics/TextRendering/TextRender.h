#pragma once

#include <string>
#include "Utilities/Singleton.h"
#include "Graphics/Renderable/Renderable.h"
#include "resourcemanager/Resourcemanager.h"

namespace GFX {

    struct Font {

        ~Font();
        void LoadFont(std::string const& path);

        struct Character {
            GLuint      mTextureID;
            glm::ivec2  mSize;
            glm::ivec2  mBearing;
            unsigned    mAdvance;
        };

        static const unsigned height = 60;                  // Height in pixels used for generating the font
        std::map<unsigned char, Character> mCharacters;     // Information about the characters in the font
        GLuint mVAO = 0, mVBO = 0, mEBO;                    // Quad used for each character
        bool mProperlyLoaded = false;

    private:
        void ClearData();
        void DeleteQuad();
        void GenerateQuad();
    };
    using FontRes = std::shared_ptr<TResource<Font>>;
}




class TranslationManager 
{
    MAKE_SINGLETON(TranslationManager)

public:
    enum class Languages {
        ENGLISH, SPANISH, TOTAL
    };
    const static std::array<std::string, (unsigned)Languages::TOTAL> LanguageNames;

    struct Message {
        std::string mVersions[(unsigned)Languages::TOTAL];
    };

    using MessageMap = std::map<unsigned, Message>;

    void                ChangeLanguage(Languages _lang);
    void                ChangeLanguage(std::string& _lang);
    std::string         GetMessage(unsigned id) const;
    MessageMap const&   GetAllMessages() const { return mMessages; }

    void                ToJson() const;
    void                FromJson();

    Languages           mCurrLanguage = Languages::ENGLISH;

#ifdef EDITOR
    void Edit();
    void PickText(unsigned* _out) const;
#endif

private:
    MessageMap          mMessages;
    unsigned            mHighestID = 0;
    const std::string   mSavePath = "./../Resources/Texts.json";
};
#define TranslationMgr (TranslationManager::Instance())




class TextComponent : public IRenderable
{
public:
    void Render() override;
    IComp* Clone() override;

#ifdef EDITOR
    bool Edit() override;
#endif

    enum class Alignment {
        Left, Centered, TOTAL
    };
    const static std::array<std::string, (unsigned)Alignment::TOTAL> mAlignmentNames;

    void SetText(std::string msg);
    void set_mode(bool _text_mode) { use_internal_message = _text_mode; }
protected:
    void ToJson(nlohmann::json& j) const;
    void FromJson(nlohmann::json& j);

private:
    GFX::FontRes    mFont = ResourceMgr.GetResource<GFX::Font>("./../Resources/Font/xd.ttf");
    unsigned        mMessageID = 0;
    Alignment       mAlignment = Alignment::Left;
    glm::vec4       mColor{};
    glm::vec3       mOffset {0.0f};
    glm::vec2       mScale {1.0f};
    bool            mbHorizontalBill = true;
    bool            mbVerticalBill = false;
    std::string     current_message{};
    bool            use_internal_message = false;
};
