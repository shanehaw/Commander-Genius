/*
 * CVideoSettings.cpp
 *
 *  Created on: 28.11.2009
 *      Author: gerstrong
 */

#include <base/CInput.h>
#include <base/GsTimer.h>
#include <base/video/CVideoDriver.h>
#include <base/interface/StringUtils.h>
#include <widgets/GsMenuController.h>
#include <base/interface/Utils.h>
#include <engine/core/GameEngine.h>
#include "engine/core/VGamepads/vgamepadsimple.h"

#include "GameSpecSettings.h"

#include "widgets/NumberControl.h"
#include "widgets/ComboSelection.h"

#include "engine/core/CSettings.h"


GameSpecSettings::GameSpecSettings(const Style style, const std::string_view &engine_name) :
#if defined(EMBEDDED)
GameMenu(GsRect<float>(0.15f, 0.20f, 0.65f, 0.25f), style ),
#else
GameMenu(GsRect<float>(0.15f, 0.20f, 0.65f, 0.55f), style ),
#endif
mEngineName(engine_name)
{
    mpGameResSelection =
        mpMenuDialog->add( new ComboSelection( "GameRes", filledStrList(1, "?x?"), style) );

    mpFPSSelection =
        mpMenuDialog->add( new NumberControl( "FPS", 10, 120, 10, 60, false, style) );


    mpBorderColorSwitch =
        mpMenuDialog->add( new Switch( "Brdr Color", style ) );

    mpHorizBordersSelection =
        mpMenuDialog->add( new NumberControl( "H-Brdr", 0, 80, 5, 0, false, style) );

    setMenuLabel("OPTIONSMENULABEL");

    mpMenuDialog->fit();

    refresh();
    select(1);
}

void GameSpecSettings::refresh()
{
    mUsersConf = gVideoDriver.getVidConfig();

    // Load the config into the GUI
    // TODO: Temporary. This must become a float later...
    const auto iFPS = static_cast<int>( gTimer.FPS() );

    // Only enable this option when VSync is turned off
    mpFPSSelection->enable(!mUsersConf.mVSync);

    mpFPSSelection->setSelection( iFPS );

    // TODO: find a way to indicate a color
    mpBorderColorSwitch->enable( mUsersConf.mBorderColorsEnabled );

    mpHorizBordersSelection->setSelection( mUsersConf.mHorizBorders );


    // Game resolution vs launcher resolution. The Launcher may use your full resolution
    const std::set<std::string> gamesResSet =
        (mEngineName == "Launcher") ?
            gVideoDriver.getResolutionStrSet() : gVideoDriver.getGameResStrSet() ;

    mpGameResSelection->setList( gamesResSet );

    //gSettings.saveGameSpecSettings(std::string_view(mEngineName));
    gSettings.loadGameSpecSettings(std::string_view(mEngineName), mUsersConf);

    std::string resStr;
    resStr = itoa(mUsersConf.mGameRect.dim.x);
    resStr += "x";
    resStr += itoa(mUsersConf.mGameRect.dim.y);
    mpGameResSelection->setSelection(resStr);
}


void GameSpecSettings::release()
{
    // Save up the changed stuff

    const auto fpsf = float(mpFPSSelection->getSelection());
    gTimer.setFPS( fpsf );

    mUsersConf.mHorizBorders = mpHorizBordersSelection->getSelection();

    mUsersConf.mBorderColorsEnabled = mpBorderColorSwitch->isEnabled();

    const std::string GameResStr = mpGameResSelection->getSelection();
    sscanf( GameResStr.c_str(), "%hux%hux",
            &mUsersConf.mGameRect.dim.x, &mUsersConf.mGameRect.dim.y );


#if defined(CAANOO) || defined(WIZ) || defined(DINGOO) || defined(NANONOTE) || defined(ANDROID)
    mUsersConf.mDisplayRect.dim.x = 320;
    mUsersConf.mDisplayRect.dim.y = 200;
#endif

    // TODO: Find a better way to setup colors in the menu
    if(mpBorderColorSwitch->isEnabled())
    {
        mUsersConf.mBorderColors.r = 0x00;
        mUsersConf.mBorderColors.g = 0xAA;
        mUsersConf.mBorderColors.b = 0xAA;
    }

    mUsersConf.mBorderColorsEnabled = mpBorderColorSwitch->isEnabled();

    mUsersConf.mHorizBorders = mpHorizBordersSelection->getSelection();

    // In case the user changed something in the camera settings, reload that.
    mUsersConf.m_CameraBounds = gVideoDriver.getCameraBounds();

    CVidConfig oldVidConf = gVideoDriver.getVidConfig();

    if(oldVidConf == mUsersConf)
        return;

    gVideoDriver.setVidConfig(mUsersConf);

    // At this point we also must apply and save the settings
    if( !gVideoDriver.applyMode() )
    {
        gSettings.loadDrvCfg(); // If it fails load the old settings
        return;
    }

    if( !gVideoDriver.start() ) // Here the same situation
    {
        gVideoDriver.setVidConfig(oldVidConf);
        gVideoDriver.start();
    }

#if defined(USE_VIRTUALPAD)
	// gVideoDriver.start when using CSDLVideoEngine, will recreate the SDLRenderer
	// which means that the textures that were created with the renderer that has now
	// been destroyed are no longer valid and SDL will silently fail to render the VirtPad.
	// This recreates the vgampad, which will trigger it to recreate the button textures again
	// using the new renderer

	 VirtualKeenControl *vkc = dynamic_cast<VirtualKeenControl*>(gInput.mpVirtPad.get());
	 if(vkc)
	 {
	 	 //Save visibility states
	 	bool dpadVis = !vkc->mDPad.invisible;
         bool confirmVis = !vkc->mConfirmButton.invisible;
         bool startVis = !vkc->mStartButton.invisible;
         bool jumpVis = !vkc->mJumpButton.invisible;
         bool pogoVis = !vkc->mPogoButton.invisible;
         bool shootVis = !vkc->mShootButton.invisible;
         bool statusVis = !vkc->mStatusButton.invisible;
         bool menuVis = !vkc->mMenuButton.invisible;

	 	gInput.mpVirtPad.reset(new VirtualKeenControl);
	 	gInput.mpVirtPad->init();
	 	vkc = dynamic_cast<VirtualKeenControl*>(gInput.mpVirtPad.get());

	 	 //Restore visibility states
	 	vkc->mDPad.invisible = !dpadVis;
         vkc->mConfirmButton.invisible = !confirmVis;
         vkc->mStartButton.invisible = !startVis;
         vkc->mJumpButton.invisible = !jumpVis;
         vkc->mPogoButton.invisible = !pogoVis;
         vkc->mShootButton.invisible = !shootVis;
         vkc->mStatusButton.invisible = !statusVis;
         vkc->mMenuButton.invisible = !menuVis;
	 }
	
	
#endif

    gEventManager.add( new SetNativeResolutionEv() );

    gSettings.saveGameSpecSettings(std::string_view(mEngineName));
    gSettings.saveDrvCfg();
    gMenuController.updateGraphics();
    gVideoDriver.setRefreshSignal(true);
}
