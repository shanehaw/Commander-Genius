
/*
 * Display.cpp
 *
 *  Created on: 09.01.2018
 *      Author: gerstrong
 */

#include <base/CInput.h>
#include <base/GsTimer.h>
#include <base/video/CVideoDriver.h>
#include <base/interface/StringUtils.h>
#include <widgets/GsMenuController.h>
#include <engine/core/GameEngine.h>

//#include "engine/core/CBehaviorEngine.h"
#include <base/interface/Utils.h>

#include "widgets/ComboSelection.h"

#include "engine/core/CSettings.h"
#include "engine/core/CMap.h"
#include "engine/core/CBehaviorEngine.h"
#include "engine/core/VGamepads/vgamepadsimple.h"
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include "DisplaySettings.h"


DisplaySettings::DisplaySettings(const Style style) :

#if TARGET_OS_IOS
	GameMenu(GsRect<float>(0.15f, 0.20f, 0.65f, 0.70f), style )
#elif defined(EMBEDDED)
	GameMenu(GsRect<float>(0.15f, 0.20f, 0.65f, 0.25f), style )
#else
	GameMenu(GsRect<float>(0.15f, 0.20f, 0.65f, 0.55f), style )
#endif
{

#if !defined(EMBEDDED) && !TARGET_OS_IOS
    mpTiltScreenSwitch =
        mpMenuDialog->add( new Switch("TiltedScr", style) );
#endif // !defined(EMBEDDED)


    mpAspectSelection =
            mpMenuDialog->add( new ComboSelection( "Aspect",
                                                          filledStrList(1, "disabled"),
                                                          style ) );


    mpFilterSelection =
            mpMenuDialog->add( new ComboSelection( "Filter",
                                                  filledStrList( 4,
                                                                 "none",
                                                                 "scale2x",
                                                                 "scale3x",
                                                                 "scale4x" ),
                                                  style ) );

    mpResolutionSelection =
        mpMenuDialog->add( new ComboSelection( "Size",
                                                  filledStrList(1, "?x?"),

                                               style) );
#if !defined(EMBEDDED)
    mpFullScreenSwitch =
        mpMenuDialog->add( new Switch( "Fullscreen", style ) );
#endif

    mpIntegerScalingSwitch =
        mpMenuDialog->add( new Switch( "IntScaling", style ) );



    mpVSyncSwitch =
        mpMenuDialog->add( new Switch( "VSync", style ) );



#if defined(USE_OPENGL)
    mpOpenGLSwitch =
        mpMenuDialog->add( new Switch( "OpenGL", style ) );

    mpRenderScaleQualitySel =
            mpMenuDialog->add( new ComboSelection( "Quality",
                                                          filledStrList( 2,
                                                                         "nearest",
                                                                         "linear" ),
                                                          style) );
#else
    mpRenderScaleQualitySel =
            mpMenuDialog->add( new ComboSelection( "Quality",
                                                          filledStrList( 3,
                                                                         "nearest",
                                                                         "linear" ),
                                                          style) );
#endif

    setMenuLabel("OPTIONSMENULABEL");

    mpMenuDialog->fit();
    refresh();
    select(1);
}

void DisplaySettings::refresh()
{
    // Copy current config to my new Config.
    // The change are taken from the menu settings
    mMyNewConf = gVideoDriver.getVidConfig();

#if !defined(EMBEDDED)
    mpTiltScreenSwitch->enable( mMyNewConf.mTiltedScreen );
#endif // !defined(EMBEDDED)


    const std::string oglFilter =
            (mMyNewConf.mRenderScQuality == CVidConfig::RenderQuality::LINEAR) ?
            "linear" : "nearest";

    mpRenderScaleQualitySel->setSelection(oglFilter);




#if defined(USE_OPENGL)
    mpOpenGLSwitch->enable( mMyNewConf.mOpengl );
#endif


    const auto aspSet = gVideoDriver.getAspectStrSet();
    mpAspectSelection->setList( aspSet );
    std::string arcStr;
    arcStr = itoa(mMyNewConf.mAspectCorrection.dim.x);
    arcStr += ":";
    arcStr += itoa(mMyNewConf.mAspectCorrection.dim.y);

    if( arcStr == "0:0")
      arcStr = "disabled";

    mpAspectSelection->setSelection(arcStr);


    mpFilterSelection->setSelection( mMyNewConf.m_ScaleXFilter==VidFilter::NONE ? "none" :
                                    (mMyNewConf.m_normal_scale ? "normal" : "scale") +
                                    itoa(int(mMyNewConf.m_ScaleXFilter)) + "x" );

#if !defined(EMBEDDED)
    mpFullScreenSwitch->enable(mMyNewConf.mFullscreen);
#endif

    mpIntegerScalingSwitch->enable(mMyNewConf.mIntegerScaling);


    const auto resList = gVideoDriver.getResolutionStrSet();
    mpResolutionSelection->setList( resList );

    std::string resStr;

    resStr = itoa(mMyNewConf.mDisplayRect.dim.x);
    resStr += "x";
    resStr += itoa(mMyNewConf.mDisplayRect.dim.y);
    mpResolutionSelection->setSelection(resStr);



    mpVSyncSwitch->enable( mMyNewConf.mVSync );

}


void DisplaySettings::release()
{
    
    // Copy current config to my new Config.
    mMyNewConf = gVideoDriver.getVidConfig();
    
#if !defined(EMBEDDED)
    mMyNewConf.mTiltedScreen = mpTiltScreenSwitch->isEnabled();
#endif // !defined(EMBEDDED)

    // Render Quality
    const std::string oglFilter = mpRenderScaleQualitySel->getSelection();
	mMyNewConf.mRenderScQuality =
		(oglFilter == "linear") ?
		CVidConfig::RenderQuality::LINEAR :
		CVidConfig::RenderQuality::NEAREST;

#if defined(USE_OPENGL)
    // OpenGL Flag
    mMyNewConf.mOpengl = mpOpenGLSwitch->isEnabled();
#else
    mMyNewConf.mOpengl = false;
#endif



    // Read Aspect correction string
    {
        const std::string arcStr = mpAspectSelection->getSelection();

        const int numRead = sscanf(arcStr.c_str(),"%i:%i",
                               &mMyNewConf.mAspectCorrection.dim.x,
                               &mMyNewConf.mAspectCorrection.dim.y);

        if(numRead < 2)
        {
            mMyNewConf.mAspectCorrection.dim.x = 0;
            mMyNewConf.mAspectCorrection.dim.y = 0;
        }
    }

    // Filter
    {
        mMyNewConf.m_ScaleXFilter = VidFilter::NONE;

        const std::string filterText = mpFilterSelection->getSelection();

        if(filterText == "normal2x")
        {
            mMyNewConf.m_normal_scale = true;
            mMyNewConf.m_ScaleXFilter = VidFilter::SCALE_2X;
        }
        if(filterText == "normal3x")
        {
            mMyNewConf.m_normal_scale = true;
            mMyNewConf.m_ScaleXFilter = VidFilter::SCALE_3X;
        }
        if(filterText == "normal4x")
        {
            mMyNewConf.m_normal_scale = true;
            mMyNewConf.m_ScaleXFilter = VidFilter::SCALE_4X;
        }
        if(filterText == "scale2x")
        {
            mMyNewConf.m_normal_scale = false;
            mMyNewConf.m_ScaleXFilter = VidFilter::SCALE_2X;
        }
        if(filterText == "scale3x")
        {
            mMyNewConf.m_normal_scale = false;
            mMyNewConf.m_ScaleXFilter = VidFilter::SCALE_3X;
        }
        if(filterText == "scale4x")
        {
            mMyNewConf.m_normal_scale = false;
            mMyNewConf.m_ScaleXFilter = VidFilter::SCALE_4X;
        }
    }

#if !defined(EMBEDDED)
    // Fullscreen
    mMyNewConf.mFullscreen = mpFullScreenSwitch->isEnabled();
#endif

    // Integer Scaling
    mMyNewConf.mIntegerScaling = mpIntegerScalingSwitch->isEnabled();

    // Read correct resolution
    {
        const std::string resStr = mpResolutionSelection->getSelection();
        int w, h;

        const int numRead = sscanf(resStr.c_str(),"%ix%i", &w, &h);
        if(numRead == 2)
        {
            GsVec2D<Uint16> res = {w, h};
            mMyNewConf.setResolution(res);
        }
    }

    // In case the user changed something in the camera settings, reload that.
    mMyNewConf.m_CameraBounds = gVideoDriver.getCameraBounds();

    // Vsync
    mMyNewConf.mVSync = mpVSyncSwitch->isEnabled();

    CVidConfig oldVidConf = gVideoDriver.getVidConfig();

    if(oldVidConf == mMyNewConf)
        return;

    gVideoDriver.setVidConfig(mMyNewConf);

    // At this point we also must apply and save the settings
    if( !gVideoDriver.applyMode() )
    {
        gSettings.loadDrvCfg(); // If it fails load the old settings
        return;
    }

	if(!gVideoDriver.start() ) // Here the same situation
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
	//


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
	// gInput.mpVirtPad.reset();
#endif

	gEventManager.add( new SetNativeResolutionEv() );

    gSettings.saveDrvCfg();
    gMenuController.updateGraphics();
    gVideoDriver.setRefreshSignal(true);


}

