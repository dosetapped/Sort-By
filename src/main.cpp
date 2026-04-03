#include <Geode/Geode.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp> 
#include <algorithm>          

using namespace geode::prelude;

// ==========================================
// 1. THE CORE SORTING ENGINE
// ==========================================
// We make this a standalone function so it never argues with Geode classes
void performSort(LevelBrowserLayer* layer, std::function<bool(GJGameLevel*, GJGameLevel*)> compareFunc) {
    auto llm = LocalLevelManager::sharedState();
    auto levels = llm->m_localLevels;

    if (!levels) return;

    // Convert GD array to C++ vector for sorting
    std::vector<GJGameLevel*> levelVec;
    for (int i = 0; i < levels->count(); ++i) {
        levelVec.push_back(static_cast<GJGameLevel*>(levels->objectAtIndex(i)));
    }

    // Sort it
    std::sort(levelVec.begin(), levelVec.end(), compareFunc);

    // Repopulate the GD array
    levels->removeAllObjects();
    for (auto level : levelVec) {
        levels->addObject(level);
    }

    // Reload the layer to show the new order
    layer->loadPage(layer->m_searchObject);
}


// ==========================================
// 2. THE LEVEL BROWSER HOOK
// ==========================================
class $modify(MyLevelSortingLayer, LevelBrowserLayer) {
    
    // --- Sorting Rules ---
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
            return a->m_levelLength < b->m_levelLength;
        });
        this->closeAlert(sender);
    }

    // --- Helper to close the popup after clicking a button ---
    void closeAlert(CCObject* sender) {
        // Climb up the UI tree to find the popup window and close it
        CCNode* current = static_cast<CCNode*>(sender);
        while (current && !typeinfo_cast<FLAlertLayer*>(current)) {
            current = current->getParent();
        }
        if (auto alert = typeinfo_cast<FLAlertLayer*>(current)) {
            alert->keyBackClicked(); 
        }
    }

    // --- Building the UI ---
    void onSortButtonClicked(CCObject* sender) {
        // Create a vanilla Geometry Dash popup with a blank description
        auto alert = FLAlertLayer::create("Sort Levels", " ", "Cancel");
        
        // Create our menu
        auto menu = CCMenu::create();
        menu->setLayout(ColumnLayout::create()->setGap(10.f)); 
        
        auto alphaBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Alphabetical"),
            this, menu_selector(MyLevelSortingLayer::onSortAlphabetical)
        );
        menu->addChild(alphaBtn);

        auto lengthBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Length"),
            this, menu_selector(MyLevelSortingLayer::onSortLength)
        );
        menu->addChild(lengthBtn);

        menu->updateLayout();
        
        // Position our menu in the center of the popup
        menu->setPosition(alert->m_mainLayer->getContentSize() / 2);
        
        // Shift it down slightly so it doesn't overlap the "Sort Levels" title
        menu->setPositionY(menu->getPositionY() - 15.f);

        // Inject our menu into the popup and show it!
        alert->m_mainLayer->addChild(menu);
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