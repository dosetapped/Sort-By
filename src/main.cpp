#include <Geode/Geode.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp> 
#include <algorithm>          

using namespace geode::prelude;

void performSort(LevelBrowserLayer* layer, std::function<bool(GJGameLevel*, GJGameLevel*)> compareFunc) {
    auto llm = LocalLevelManager::sharedState();
    auto levels = llm->m_localLevels;

    if (!levels) return;

    std::vector<GJGameLevel*> levelVec;
    for (int i = 0; i < levels->count(); ++i) {
        levelVec.push_back(static_cast<GJGameLevel*>(levels->objectAtIndex(i)));
    }

    std::sort(levelVec.begin(), levelVec.end(), compareFunc);

    levels->removeAllObjects();
    for (auto level : levelVec) {
        levels->addObject(level);
    }

    layer->loadPage(layer->m_searchObject);
}

class $modify(MyLevelSortingLayer, LevelBrowserLayer) {
    
    void onSortAlphabetical(CCObject* sender) {
        performSort(this, [](GJGameLevel* a, GJGameLevel* b) {
            std::string nameA = a->m_levelName;
            std::string nameB = b->m_levelName;
            std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
            std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);
            return nameA < nameB; 
        });
        this->closeAlert(sender);
    }

    void onSortLength(CCObject* sender) {
        performSort(this, [](GJGameLevel* a, GJGameLevel* b) {
            return a->m_levelLength > b->m_levelLength; 
        });
        this->closeAlert(sender);
    }

    void onSortTimeSpent(CCObject* sender) {
        performSort(this, [](GJGameLevel* a, GJGameLevel* b) {
            return a->m_workingTime > b->m_workingTime; 
        });
        this->closeAlert(sender);
    }

    void closeAlert(CCObject* sender) {
        CCNode* current = static_cast<CCNode*>(sender);
        while (current && !typeinfo_cast<FLAlertLayer*>(current)) {
            current = current->getParent();
        }
        if (auto alert = typeinfo_cast<FLAlertLayer*>(current)) {
            alert->keyBackClicked(); 
        }
    }

    void onSortButtonClicked(CCObject* sender) {
        auto alert = FLAlertLayer::create("Sort Levels", "\n\n\n\n\n\n\n\n", "Cancel");
        
        auto alphaBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Alphabetical"),
            this, menu_selector(MyLevelSortingLayer::onSortAlphabetical)
        );
        auto lengthBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Length"),
            this, menu_selector(MyLevelSortingLayer::onSortLength)
        );
        auto timeBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Time Spent"),
            this, menu_selector(MyLevelSortingLayer::onSortTimeSpent)
        );

        alphaBtn->setPosition({0, 160.f});
        lengthBtn->setPosition({0, 105.f});
        timeBtn->setPosition({0, 50.f});

        alert->m_buttonMenu->addChild(alphaBtn);
        alert->m_buttonMenu->addChild(lengthBtn);
        alert->m_buttonMenu->addChild(timeBtn);
        
        alert->show();
    }

    bool init(GJSearchObject* searchObj) {
        if (!LevelBrowserLayer::init(searchObj)) {
            return false;
        }

        if (searchObj->m_searchType == SearchType::MyLevels) {
            auto sortButton = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Sort"),
                this, menu_selector(MyLevelSortingLayer::onSortButtonClicked)
            );
            sortButton->setID("sort-button"_spr);

            if (auto menu = this->getChildByIDRecursive("new-level-menu")) {
                menu->addChild(sortButton);
                menu->updateLayout(); 
            } 
            else {
                auto fallbackMenu = CCMenu::create();
                fallbackMenu->setPosition({ 60.f, 60.f }); 
                fallbackMenu->addChild(sortButton);
                this->addChild(fallbackMenu);
            }
        }
        return true;
    }
};