// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "BT_Common.h"
#include "BT_CustomActor.h"
#include "BT_DialogManager.h"
#include "BT_AnimationManager.h"
#include "BT_AutomatedTradeshowDemo.h"
#include "BT_Camera.h"
#include "BT_EventManager.h"
#include "BT_FacebookWidgetJavaScriptAPI.h"
#include "BT_FileSystemManager.h"
#include "BT_JavaScriptAPI.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif
#include "BT_PbPersistenceHelpers.h"
#include "BT_PhotoFrameActor.h"
#include "BT_Pile.h"
#include "BT_RaycastReports.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WatchedObjects.h"
#include "BT_WebActor.h"
#include "BT_WindowsOS.h"
#include "BumpTop.pb.h"
#include "TwitterClient.h"
#include "FacebookClient.h"

//
// RestoreOriginalPhoto control implementation
// 
RestoreOriginalPhotoControl::RestoreOriginalPhotoControl()
: _layout(NULL)
, _hLayout(NULL)
, _label(NULL)
, _enabled(false)
{}

RestoreOriginalPhotoControl::~RestoreOriginalPhotoControl()
{}

void RestoreOriginalPhotoControl::init()
{
	_layout = new OverlayLayout;
	_layout->getStyle().setOffset(Vec3(-0.13f, -0.05f, 0));
	_label = new TextOverlay(QT_TR_NOOP("Undo Photo Crop"));
	_label->setAlpha(1.0f);
	_hLayout = new HorizontalOverlayLayout;
	_hLayout->getStyle().setBackgroundColor(ColorVal(60, 0, 0, 0));
	_hLayout->getStyle().setPadding(AllEdges, 5.0f);
	_hLayout->getStyle().setPadding(RightEdge, 10.0f);
	_hLayout->getStyle().setSpacing(2.0f);
	_hLayout->addItem(_label);
	_hLayout->addMouseEventHandler(this);
	_layout->addItem(_hLayout);
	scnManager->registerOverlay(_layout);
}

void RestoreOriginalPhotoControl::show()
{
	if (_enabled)
		return;

	_enabled = true;

	// If the control is not created, then create it
	// This is lazy initialization.
	if (!_layout)
	{
		init();	
	}

	fadeIn();
	return;
}

bool RestoreOriginalPhotoControl::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;

	// Zoom to default camera position
	FileSystemActor* photo = dynamic_cast<FileSystemActor*>(cam->getHighlightedWatchedActor());
	restoreOriginalPhoto(photo);
	return true;
}

bool RestoreOriginalPhotoControl::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;
	return true;
}

bool RestoreOriginalPhotoControl::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	return false;
}

void RestoreOriginalPhotoControl::hide()
{
	if (_enabled)
	{
		fadeOut();
		_enabled = false;
	}
}

void RestoreOriginalPhotoControl::fadeIn()
{
	_hLayout->finishAnimation();
	float alpha = _hLayout->getStyle().getAlpha();
	if (alpha < 1.0f)
		_hLayout->setAlphaAnim(alpha, 1.0f, 15);
}

void RestoreOriginalPhotoControl::fadeOut()
{	
	_hLayout->finishAnimation();
	float alpha = _hLayout->getStyle().getAlpha();
	if (alpha > 0.0f)
		_hLayout->setAlphaAnim(alpha, 0.0f, 15);
}

//
// CameraOverlay implementation
//
void SlideShowControls::init()
{
	_enabled = true;

	_facebookActor = getFacebookActor(false);

	if (_slideshowMenuContainer) //Already initialized
	{
		fadeIn();
		return;
	}

	//Create Slideshow Menu Items
	_slideshowMenuContainer = new OverlayLayout;
	_slideshowMenuHLayout = new HorizontalOverlayLayout;

	_twitterContainer = new OverlayLayout;
	_twitterLayout = new VerticalOverlayLayout;
	_twitterImage = new ImageOverlay("pui.twitter");
	_twitterLabel = new TextOverlay(QT_TR_NOOP("Twitter"));

	_facebookContainer = new OverlayLayout;
	_facebookLayout = new VerticalOverlayLayout;
	_facebookImage = new ImageOverlay("pui.facebook");
	_facebookLabel = new TextOverlay(QT_TR_NOOP("Facebook"));

	_emailContainer = new OverlayLayout;
	_emailLayout = new VerticalOverlayLayout;
	_emailImage = new ImageOverlay("pui.email");
	_emailLabel = new TextOverlay(QT_TR_NOOP("Email"));

	_printerContainer = new OverlayLayout;
	_printerLayout = new VerticalOverlayLayout;
	_printerImage = new ImageOverlay("pui.print");
	_printerLabel = new TextOverlay(QT_TR_NOOP("Print"));

	_editContainer = new OverlayLayout;
	_editLayout = new VerticalOverlayLayout;
	_editImage = new ImageOverlay("pui.edit");
	_editLabel = new TextOverlay(QT_TR_NOOP("Edit"));

	_dividerImage1 = new ImageOverlay("pui.divider");
	_dividerImage2 = new ImageOverlay("pui.divider");
	_dividerImage3 = new ImageOverlay("pui.divider");
	_dividerImage4 = new ImageOverlay("pui.divider");
	_dividerImage5 = new ImageOverlay("pui.divider");
	_dividerImage6 = new ImageOverlay("pui.divider");

	_closeContainer = new OverlayLayout;
	_closeLayout = new VerticalOverlayLayout;
	_closeImage = new ImageOverlay("pui.close");
	_closeLabel = new TextOverlay(QT_TR_NOOP("Close"));

	_nextContainer = new OverlayLayout;
	_nextLayout = new VerticalOverlayLayout;
	_nextImage = new ImageOverlay("pui.next");

	_prevContainer = new OverlayLayout;
	_prevLayout = new VerticalOverlayLayout;
	_prevImage = new ImageOverlay("pui.previous");

	//Initialize Slideshow Menu Items
	_slideshowMenuContainer->getStyle().setOffset(Vec3(-0.5f, 0.02f, 0));
	_slideshowMenuHLayout->getStyle().setBackgroundColor(ColorVal(195, 0, 0, 0));
	_slideshowMenuHLayout->getStyle().setPadding(AllEdges,5.0f);

	FontDescription slideShowButtonFont(QT_NT("Tahoma bold"), 10);
	_twitterLabel->setFont(slideShowButtonFont);
	_facebookLabel->setFont(slideShowButtonFont);
	_emailLabel->setFont(slideShowButtonFont);
	_printerLabel->setFont(slideShowButtonFont);
	_editLabel->setFont(slideShowButtonFont);
	_closeLabel->setFont(slideShowButtonFont);

	_twitterLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_facebookLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_emailLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_printerLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_editLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_closeLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_nextLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	_prevLayout->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));

	_twitterLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_twitterLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_twitterLayout->getStyle().setSpacing(0.0f);

	_facebookLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_facebookLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_facebookLayout->getStyle().setSpacing(0.0f);

	_emailLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_emailLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_emailLayout->getStyle().setSpacing(0.0f);

	_printerLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_printerLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_printerLayout->getStyle().setSpacing(0.0f);

	_editLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_editLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_editLayout->getStyle().setSpacing(0.0f);

	_closeLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_closeLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_closeLayout->getStyle().setSpacing(0.0f);

	_nextLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_nextLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_nextLayout->getStyle().setSpacing(0.0f);

	_prevLayout->getStyle().setPadding(TopBottomEdges, 10.0f);
	_prevLayout->getStyle().setPadding(LeftRightEdges, 0.0f);
	_prevLayout->getStyle().setSpacing(0.0f);

	_dividerImage1->getStyle().setPadding(TopBottomEdges, 10.0f);
	_dividerImage1->getStyle().setPadding(LeftRightEdges, 0.0f);
	_dividerImage1->getStyle().setSpacing(0.0f);

	_dividerImage2->getStyle().setPadding(TopBottomEdges, 10.0f);
	_dividerImage2->getStyle().setPadding(LeftRightEdges, 0.0f);
	_dividerImage2->getStyle().setSpacing(0.0f);

	_dividerImage3->getStyle().setPadding(TopBottomEdges, 10.0f);
	_dividerImage3->getStyle().setPadding(LeftRightEdges, 0.0f);
	_dividerImage3->getStyle().setSpacing(0.0f);

	_dividerImage4->getStyle().setPadding(TopBottomEdges, 10.0f);
	_dividerImage4->getStyle().setPadding(LeftRightEdges, 0.0f);
	_dividerImage4->getStyle().setSpacing(0.0f);

	_dividerImage5->getStyle().setPadding(TopBottomEdges, 10.0f);
	_dividerImage5->getStyle().setPadding(LeftRightEdges, 0.0f);
	_dividerImage5->getStyle().setSpacing(0.0f);

	_dividerImage6->getStyle().setPadding(TopBottomEdges, 10.0f);
	_dividerImage6->getStyle().setPadding(LeftRightEdges, 0.0f);
	_dividerImage6->getStyle().setSpacing(0.0f);

	_twitterLayout->addItem(_twitterImage);
	_twitterLayout->addItem(_twitterLabel);
	_twitterContainer->addMouseEventHandler(this);

	_facebookLayout->addItem(_facebookImage);
	_facebookLayout->addItem(_facebookLabel);
	_facebookContainer->addMouseEventHandler(this);

	_emailLayout->addItem(_emailImage);
	_emailLayout->addItem(_emailLabel);
	_emailContainer->addMouseEventHandler(this);

	_printerLayout->addItem(_printerImage);
	_printerLayout->addItem(_printerLabel);
	_printerContainer->addMouseEventHandler(this);

	_editLayout->addItem(_editImage);
	_editLayout->addItem(_editLabel);
	_editContainer->addMouseEventHandler(this);

	_closeLayout->addItem(_closeImage);
	_closeLayout->addItem(_closeLabel);
	_closeContainer->addMouseEventHandler(this);

	_nextLayout->addItem(_nextImage);
	_nextContainer->addMouseEventHandler(this);

	_prevLayout->addItem(_prevImage);
	_prevContainer->addMouseEventHandler(this);

	_twitterContainer->addItem(_twitterLayout);
	_facebookContainer->addItem(_facebookLayout);
	_emailContainer->addItem(_emailLayout);
	_printerContainer->addItem(_printerLayout);
	_editContainer->addItem(_editLayout);
	_closeContainer->addItem(_closeLayout);
	_nextContainer->addItem(_nextLayout);
	_prevContainer->addItem(_prevLayout);

	/* See button hit boxes
	_twitterLabel->getStyle().setBackgroundColor(ColorVal(255,255,0,0));
	_twitterImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_twitterContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_facebookLabel->getStyle().setBackgroundColor(ColorVal(255,255,0,0));
	_facebookImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_facebookContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_emailLabel->getStyle().setBackgroundColor(ColorVal(255,255,0,0));
	_emailImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_emailContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_prevImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_prevContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_nextImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_nextContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_printerLabel->getStyle().setBackgroundColor(ColorVal(255,255,0,0));
	_printerImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_printerContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_editLabel->getStyle().setBackgroundColor(ColorVal(255,255,0,0));
	_editImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_editContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));

	_closeLabel->getStyle().setBackgroundColor(ColorVal(255,255,0,0));
	_closeImage->getStyle().setBackgroundColor(ColorVal(255,0,255,0));
	_closeContainer->getStyle().setBackgroundColor(ColorVal(255,0,0,255));
	/**/

	_slideshowMenuHLayout->addItem(_twitterContainer);
	_slideshowMenuHLayout->addItem(_dividerImage1);
	_slideshowMenuHLayout->addItem(_facebookContainer);
	_slideshowMenuHLayout->addItem(_dividerImage2);
	_slideshowMenuHLayout->addItem(_emailContainer);
	_slideshowMenuHLayout->addItem(_dividerImage3);
	_slideshowMenuHLayout->addItem(_prevContainer);
	_slideshowMenuHLayout->addItem(_nextContainer);
	_slideshowMenuHLayout->addItem(_dividerImage4);
	_slideshowMenuHLayout->addItem(_printerContainer);
	_slideshowMenuHLayout->addItem(_dividerImage5);
	_slideshowMenuHLayout->addItem(_editContainer);
	_slideshowMenuHLayout->addItem(_dividerImage6);
	_slideshowMenuHLayout->addItem(_closeContainer);

	_slideshowMenuContainer->addItem(_slideshowMenuHLayout);
	scnManager->registerOverlay(_slideshowMenuContainer);
	// NOTE: set the size animations _after_ they have been added to the layouts

}

SlideShowControls::SlideShowControls()
{
	twitterClient = NULL;
	twitpicClient = NULL;
	facebookClient = NULL;
	_slideshowMenuContainer = NULL;
	_slideshowMenuHLayout = NULL;
	_twitterContainer = NULL;
	_twitterLayout = NULL;
	_twitterImage = NULL;
	_twitterLabel = NULL;
	_facebookContainer = NULL;
	_facebookLayout = NULL;
	_facebookImage = NULL;
	_facebookLabel = NULL;
	_emailContainer = NULL;
	_emailLayout = NULL;
	_emailImage = NULL;
	_emailLabel = NULL;
	_printerContainer = NULL;
	_printerLayout = NULL;
	_printerImage = NULL;
	_printerLabel = NULL;
	_editContainer = NULL;
	_editLayout = NULL;
	_editImage = NULL;
	_editLabel = NULL;
	_closeContainer = NULL;
	_closeLayout = NULL;
	_closeImage = NULL;
	_closeLabel = NULL;
	_prevContainer = NULL;
	_prevLayout = NULL;
	_prevImage = NULL;
	_nextContainer = NULL;
	_nextLayout = NULL;
	_nextImage = NULL;
	_dividerImage1 = NULL;
	_dividerImage2 = NULL;
	_dividerImage3 = NULL;
	_dividerImage4 = NULL;
	_dividerImage5 = NULL;
	_dividerImage6 = NULL;
}

SlideShowControls::~SlideShowControls()
{
	// Scene manager automatically removes all unused overlays
}

void SlideShowControls::fadeIn()
{
	_slideshowMenuHLayout->setAlphaAnim(_slideshowMenuHLayout->getStyle().getAlpha(), 1.0f, 25);
}

void SlideShowControls::fadeOut()
{
	_slideshowMenuHLayout->finishAnimation();
	float alpha = _slideshowMenuHLayout->getStyle().getAlpha();

	if (alpha > 0.0f)
		_slideshowMenuHLayout->setAlphaAnim(alpha, 0.0f, 25);

	if(!evtManager->getACPowerStatus()) { //Battery power
		_slideshowMenuHLayout->setAlpha(0.0f);
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

void SlideShowControls::twitterAction() {
	BumpObject* currentActor = cam->getHighlightedWatchedActor();

	if (!ftManager->hasInternetConnection())
	{
		printUniqueError("TwitterActorImpl", QT_TRANSLATE_NOOP("TwitpicClient", "No internet connection detected"));
		return;
	}

	CustomActor * twit = scnManager->getCustomActor<TwitterActorImpl>();

	if(twit) { //Twitter actor exists

		std::vector<BumpObject *> currentActor;
		currentActor.push_back(cam->getHighlightedWatchedActor());
		twit->onDrop(currentActor);
		return;

	} else { //We need to open up our own connection to twitter (based on code from the onDrop event)

		if(!twitterClient) twitterClient = new TwitterClient(TWITTER_SERVER);
		if(!twitpicClient) twitpicClient = new TwitpicClient(TWITPIC_SERVER);

		BumpObject* currentActor = cam->getHighlightedWatchedActor();

		if (twitterClient->initialize() && twitpicClient->initialize()) {
			if (currentActor->getObjectType() == ObjectType(BumpActor, FileSystem, Image))
			{
				QString photoPath = dynamic_cast<FileSystemActor *>(currentActor)->getFullPath();

				twitpicClient->uploadAndPostDeferMessage(photoPath); //tweet
			}
			else
			{
				// non-files are rejected
				printUniqueError("TwitterActorImpl", QT_TRANSLATE_NOOP("TwitpicClient", "Unsupported image file type"));
			}
		}
	}
}
void SlideShowControls::facebookAction() {

	if (!ftManager->hasInternetConnection())
	{
		printUniqueError("FacebookActorImpl", QT_TRANSLATE_NOOP("FacebookClient", "No internet connection detected"));
		return;
	}

	bool wasOn = true;

	if(!_facebookActor) {
		_facebookActor = getFacebookActor(true);
		wasOn = false;
	}
	
	FacebookWidgetJavaScriptAPI* fbAPI = dynamic_cast<FacebookWidgetJavaScriptAPI*>(_facebookActor->getJavaScriptAPI());

	//We found the facebook widget
	if(GLOBAL(settings).fbc_uid.isEmpty() && GLOBAL(settings).fbc_session.isEmpty() && GLOBAL(settings).fbc_secret.isEmpty()) { //User is not logged in
		fbAPI->launchLoginPage();
		fbAPI->setPersistentStoreValue(QT_NT("last-page"),QT_NT("newsfeed"));
		_facebookActor->load(QT_NT("bumpwidget-facebook://newsfeed"), true);
	}

	//The reason we check twice is that the user could have cancelled the login process. 
	if(!(GLOBAL(settings).fbc_uid.isEmpty() && GLOBAL(settings).fbc_session.isEmpty() && GLOBAL(settings).fbc_secret.isEmpty())) { //User has logged in
		vector<BumpObject *> watched;
		watched.push_back(cam->getHighlightedWatchedActor());
		fbAPI->onDropEnter(watched);
		fbAPI->onDrop();

		goFacebookAction();
	} else {
		if(!wasOn) {
			FadeAndDeleteActor(_facebookActor);
		}
	}
}

WebActor * SlideShowControls::getFacebookActor(bool createIfNotExists) {
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isFacebookWidgetUrl()) 
		{
			return actor;
		}
	}

	if(createIfNotExists) {
		Key_ToggleFacebookWidget();
		vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
		for (int i = 0; i < webActors.size(); ++i) 
		{
			WebActor * actor = (WebActor *) webActors[i];
			if (actor->isFacebookWidgetUrl()) 
			{
				return actor;
			}
		}
	}

	return NULL;
}

void SlideShowControls::goFacebookAction() {
	cam->killAnimation();

	Key_ToggleSlideShow();
	_facebookActor->onLaunch();
}

void SlideShowControls::printerAction(){
	CustomActor * prnt = scnManager->getCustomActor<PrinterActorImpl>();
	if(prnt) { //Printer actor exists

		std::vector<BumpObject *> currentActor;
		currentActor.push_back(cam->getHighlightedWatchedActor());
		prnt->onDrop(currentActor);

		return;

	} else { //We need to open up our own connection to the printer (based on code from the onDrop event function)		

		BumpObject * currentActor = cam->getHighlightedWatchedActor();

		if (!(currentActor->getObjectType() == ObjectType(BumpActor, FileSystem, File)))
		{
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("You can only print files!"), Message::Ok, clearPolicy));
			return;
		}	

		dlgManager->clearState();
		dlgManager->setPrompt(QT_TR_NOOP("Print these file(s) on your default printer?"));
		if (!dlgManager->promptDialog(DialogYesNo))
		{
			dismiss("PrinterActorImpl::onDrop");
			return;
		}

		QString filePath = dynamic_cast<FileSystemActor *>(currentActor)->getFullPath();

		int result = (int) ShellExecute(winOS->GetWindowsHandle(), QT_NT(L"print"), (LPCTSTR) filePath.utf16(), NULL, NULL, SW_SHOWNORMAL);
		if (result <= 32)
		{
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("Could not print this file!"), Message::Ok, clearPolicy));
			return;
		}
		else
		{
			// notify the user
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(3);
			Message * message = new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("Printing file"), Message::Ok, clearPolicy);
			scnManager->messages()->addMessage(message);
			return;
		}

	}
}
void SlideShowControls::closeAction(){
	Key_ToggleSlideShow();
}
void SlideShowControls::emailAction(){
	CustomActor * mail = scnManager->getCustomActor<EmailActorImpl>();
	if(mail) { //Email actor exists

		std::vector<BumpObject *> currentActor;
		currentActor.push_back(cam->getHighlightedWatchedActor());
		mail->onDrop(currentActor);
		return;
	} else { //We need to open up our own connection to email (based on code from the onDrop event function)

		std::vector<BumpObject *> currentActor;
		currentActor.push_back(cam->getHighlightedWatchedActor());

		if (!CreateEmailWithSelectionAsAttachments(currentActor))
		{
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("EmailActorImpl::onDrop", QT_TR_NOOP("Could not create email with these files\nas attachments!"), Message::Ok, clearPolicy));
		}
		else
		{
			// notify the user
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(3);
			Message * message = new Message("EmailActorImpl::onDrop", QT_TR_NOOP("Creating new email"), Message::Ok, clearPolicy);
			scnManager->messages()->addMessage(message);
		}

	}
}
void SlideShowControls::nextAction(){
	cam->highlightNextWatchedActor(true);
}
void SlideShowControls::prevAction(){
	cam->highlightNextWatchedActor(false);
}
void SlideShowControls::editAction(){
	assert(cam->getHighlightedWatchedActor() != NULL);
	BumpObject * currentActor = cam->getHighlightedWatchedActor();

	if(currentActor->getObjectType() == ObjectType(BumpActor, FileSystem)) {
		fsManager->launchFile(dynamic_cast<FileSystemActor *>(currentActor)->getFullPath(),L"edit");
	}
}

int SlideShowControls::getHeight() {
	return _slideshowMenuHLayout->getSize().y;
}

void *callbackManager(AnimationEntry *anim) {

	SlideShowControls *controls = (SlideShowControls *)anim->getCustomData();

	switch (controls->pressed) {
		case SlideShowControls::TWITTER:
			controls->twitterAction();
			break;
		case SlideShowControls::FACEBOOK:
			controls->facebookAction();
			break;
		case SlideShowControls::EMAIL:
			controls->emailAction();
			break;
		case SlideShowControls::EDIT:
			controls->editAction();
			break;
		case SlideShowControls::CLOSE:
			controls->closeAction();
			break; 
		case SlideShowControls::NEXT:
			controls->nextAction();
			break;
		case SlideShowControls::PREVIOUS:
			controls->prevAction();
			break;
		case SlideShowControls::GOFACEBOOK:
			controls->goFacebookAction();
			break;
		case SlideShowControls::PRINTER:
			controls->printerAction();
			break;
	}


	return NULL;
}

bool SlideShowControls::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return true;

	//TODO: A lot of this code is parallel to the respective actors in customactor, if possible we should share these resources

	OverlayComponent * button = dynamic_cast<OverlayComponent *>(mouseEvent.getTarget());

	if (dynamic_cast<OverlayLayout *>(button) == _twitterContainer) {//Twitter button

		pressed = TWITTER;

	} else if (dynamic_cast<OverlayLayout *>(button) == _facebookContainer) {//Facebook button

		pressed = FACEBOOK;

	} else if (dynamic_cast<OverlayLayout *>(button) == _emailContainer) {//Email button

		pressed = EMAIL;

	} else if (dynamic_cast<OverlayLayout *>(button) == _printerContainer) {//Printer button

		pressed = PRINTER;

	} else if (dynamic_cast<OverlayLayout *>(button) == _editContainer) {//Edit button

		pressed = EDIT;

	} else if (dynamic_cast<OverlayLayout *>(button) == _closeContainer) {//Close button

		pressed = CLOSE;

	} else if (dynamic_cast<OverlayLayout *>(button) == _prevContainer) {//Previous image button

		pressed = PREVIOUS;

	} else if (dynamic_cast<OverlayLayout *>(button) == _nextContainer) {//Next image button

		pressed = NEXT;

	} else {

		pressed = NOPRESS;

	}

	if(pressed != NOPRESS) {
		if(button->isAnimating()) {
			button->finishAnimation();
			AnimationEntry a = AnimationEntry(button,NULL,this);
			callbackManager(&a);
		}
		button->setAlphaAnim(0.5f,1.0f,15,(FinishedCallBack)callbackManager, this);
	}
	return true;
}

bool SlideShowControls::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;

	// reset the color of the font
	OverlayComponent * button = mouseEvent.getTarget();

	button->getStyle().setColor(ColorVal(255, 255, 255, 255));
	if (!mouseEvent.intersectsTarget())
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	return true;
}

bool SlideShowControls::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;

	return false;
}

void SlideShowControls::disable()
{
	if (_slideshowMenuContainer)
	{
		fadeOut();
		_enabled = false;
	}
}


//
// ZoomOut control implementation
// 
ZoomOutControl::ZoomOutControl()
: _layout(NULL)
, _hLayout(NULL)
, _zoomOutLabel(NULL)
, _enabled(false)
{}

ZoomOutControl::~ZoomOutControl()
{}

void ZoomOutControl::init()
{
	_layout = new OverlayLayout;
	_layout->getStyle().setOffset(Vec3(-0.10f, -0.05f, 0));
	_defaultText = QT_TR_NOOP("Reset Camera");
	_zoomOutLabel = new TextOverlay(_defaultText);
	_zoomOutLabel->setAlpha(1.0f);
	_zoomOutLabel->getTextBuffer().pushFlag(TextPixmapBuffer::ForceLinearFiltering);
	_zoomOutLabel->getTextBuffer().pushFlag(TextPixmapBuffer::DisableClearType);
	_hLayout = new HorizontalOverlayLayout;
	_hLayout->getStyle().setBackgroundColor(ColorVal(60, 0, 0, 0));
	_hLayout->getStyle().setPadding(AllEdges, 5.0f);
	_hLayout->getStyle().setPadding(RightEdge, 10.0f);
	_hLayout->getStyle().setSpacing(2.0f);
	_hLayout->addItem(_zoomOutLabel);
	_hLayout->addMouseEventHandler(this);
	_layout->addItem(_hLayout);
	scnManager->registerOverlay(_layout);
}

void ZoomOutControl::show()
{
	if (_enabled)
		return;

	_enabled = true;

	// If the control is not created, then create it
	// This is lazy initialization.
	if (!_layout)
	{
		init();
	}

	fadeIn();
	return;
}

void ZoomOutControl::setText(QString text, Vec3 offset)
{
	if (!_layout)
	{
		init();
		_hLayout->finishAnimation();
		if (!_enabled) // If not visible, set alpha to 0 since init() automatically sets alpha to 1
			_hLayout->getStyle().setAlpha(0);
	}

	if (text == _zoomOutLabel->getText())
		return;
	
	if (text.isEmpty())
	{
		text = _defaultText;
		offset = Vec3(-0.10f, -0.05f, 0);
	}

	_zoomOutLabel->setText(text, false);
	_zoomOutLabel->setSize(_zoomOutLabel->getPreferredDimensions());
	_hLayout->setSize(_hLayout->getPreferredDimensions());
	_hLayout->reLayout();
	_layout->getStyle().setOffset(offset);
	_layout->reLayout();
}

bool ZoomOutControl::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;

	if (!WebActor::zoomOutFocusedWebActor()) //zoomOutFocusedWebActor will only zoom out when there is exactly one web actor focused / zoomed
	{
		cam->getZoomedObjects().clear();
		cam->loadCameraFromPreset(GLOBAL(settings.cameraPreset));
	}

	return true;
}

bool ZoomOutControl::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;
	return true;
}

bool ZoomOutControl::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	return false;
}

void ZoomOutControl::hide()
{
	if (_enabled)
	{
		cam->setTrackingWatchedActors(true);
		fadeOut();
		_enabled = false;
	}
}

bool ZoomOutControl::isEnabled()
{
	return _enabled;
}

void ZoomOutControl::fadeIn()
{
	_hLayout->finishAnimation();
	float alpha = _hLayout->getStyle().getAlpha();
	if (alpha < 1.0f)
		_hLayout->setAlphaAnim(alpha, 1.0f, 15);
}

void ZoomOutControl_restoreText(AnimationEntry & entry)
{
	((ZoomOutControl *)entry.getCustomData())->setText(QString(), Vec3(0.0f)); // Restore text to "Reset Camera"
}

void ZoomOutControl::fadeOut()
{	
	_hLayout->finishAnimation();
	float alpha = _hLayout->getStyle().getAlpha();
	if (alpha > 0.0f)
		_hLayout->setAlphaAnim(alpha, 0.0f, 15, &ZoomOutControl_restoreText, this);
}

//
// CornerInfoControl control implementation
// 
CornerInfoControl::CornerInfoControl()
: _layout(NULL)
, _hLayout(NULL)
, _zoomOutLabel(NULL)
, _enabled(false)
{}

CornerInfoControl::~CornerInfoControl()
{}

void CornerInfoControl::init()
{
	_enabled = true;
	if (_layout)
	{
		fadeIn();
		return;
	}

	_layout = new OverlayLayout;
	_layout->getStyle().setOffset(Vec3(0.0f, -0.07f, 0));
	_zoomOutLabel = new TextOverlay("");
	_zoomOutLabel->getTextBuffer().pushFlag(TextPixmapBuffer::ForceLinearFiltering);
	_zoomOutLabel->getTextBuffer().pushFlag(TextPixmapBuffer::DisableClearType);
	_hLayout = new HorizontalOverlayLayout;
	_hLayout->getStyle().setBackgroundColor(ColorVal(80, 0, 0, 0));
	_hLayout->getStyle().setPadding(AllEdges, 10.0f);
	_hLayout->getStyle().setPadding(TopEdge, 12.0f);
	_hLayout->getStyle().setCornerRadius(TopRightCorner|BottomRightCorner, 10.0f);
	_hLayout->getStyle().setCornerRadius(TopLeftCorner|BottomLeftCorner, 0.0f);
	_hLayout->getStyle().setSpacing(2.0f);
	_hLayout->addItem(_zoomOutLabel);
	_hLayout->addMouseEventHandler(this);
	_layout->addItem(_hLayout);
	scnManager->registerOverlay(_layout);
}

bool CornerInfoControl::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;

	return true;
}

bool CornerInfoControl::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	if (!_enabled)
		return false;

	// If the demo is running stop it, otherwise disable the overlay
	Replayable * replay = scnManager->getReplayable();
	if (replay != NULL)
	{
		AutomatedTradeshowDemo * demo = dynamic_cast<AutomatedTradeshowDemo *>(replay);
		if (demo->getPlayState() == replay->Running)
		{
			demo->quickStop();
			setMessageText(QT_TR_NOOP("BumpTop Demo Scene\nTo refresh the scene hit F7\nTo start the automatic demo hit CTRL + F7"));
		}
		else
			disable();
	}
	else
		disable();
	return true;
}

bool CornerInfoControl::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	return false;
}

void CornerInfoControl::disable()
{
	if (_layout)
	{
		fadeOut();
		_enabled = false;
	}
}

bool CornerInfoControl::isEnabled()
{
	return _enabled;
}

void CornerInfoControl::fadeIn()
{
	float alpha = _hLayout->getStyle().getAlpha();
	if (alpha < 1.0f)
		_hLayout->setAlphaAnim(alpha, 1.0f, 15);
}

void CornerInfoControl::fadeOut()
{	
	float alpha = _hLayout->getStyle().getAlpha();
	if (alpha > 0.0f)
		_hLayout->setAlphaAnim(alpha, 0.0f, 15);
}

void CornerInfoControl::setMessageText(QString text)
{
	if (!_zoomOutLabel) 
		return;

	_zoomOutLabel->setText(text);
	_hLayout->setSize(_hLayout->getPreferredDimensions());
}



//
// Camera implementation
//
const float Camera::SAME_POSITION_THRESHOLD = 0.0005f;

Camera::Camera()
: 
#ifndef DXRENDER
_glProjMatrix(new GLdouble[16])
, _glMVMatrix(new GLdouble[16])
, _glViewport(new GLint[4])
, _isglDataDirty(false)
, 
#endif
_animationFinishData(NULL)
, _currentCameraView(DefaultView)
{
	_animationFinishCallback = NULL;
	// Default Values
	setOrientation(Vec3(0, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1));
	cameraIsFreeForm = false;

#ifndef DXRENDER
	ZeroMemory(_glProjMatrix, 16 * sizeof(GLdouble));
	ZeroMemory(_glMVMatrix, 16 * sizeof(GLdouble));
	ZeroMemory(_glViewport, 4 * sizeof(GLint));
#endif

	// init the visible bounds
	visibleBounds.setEmpty();
}

Camera::~Camera()
{
#ifndef DXRENDER
	SAFE_DELETE(_glProjMatrix);
	SAFE_DELETE(_glMVMatrix);
	SAFE_DELETE(_glViewport);
#endif 
	popAllWatchActors();
}

Vec3 Camera::getEye()
{
	return eye;
}

Vec3 Camera::getUp()
{
	return up;
}

Vec3 Camera::getDir()
{
	return dir;
}

void Camera::setEye(Vec3 newEye)
{
	eye = newEye;
#ifndef DXRENDER
	_isglDataDirty = true;
#endif
}

void Camera::setDir(Vec3 newDir)
{
	dir = newDir;
	dir.normalize();
#ifndef DXRENDER
	_isglDataDirty = true;
#endif
}

void Camera::setUp(Vec3 newUp)
{
	up = newUp;
#ifndef DXRENDER
	_isglDataDirty = true;
#endif
}

// Camera settings loaded from Scene file
Vec3 Camera::getSavedEye()
{
	return savedEye;
}

Vec3 Camera::getSavedDir()
{
	return savedDir;
}

Vec3 Camera::getSavedUp()
{
	return savedUp;
}

void Camera::storeCameraPosition(Vec3 newEye, Vec3 newDir, Vec3 newUp)
{
	savedEye = newEye;
	savedDir = newDir;
	savedUp = newUp;
}

bool Camera::isCameraFreeForm()
{
	return cameraIsFreeForm;
}

void Camera::setIsCameraFreeForm(bool isCameraFreeForm, bool showMessage)
{
	if (!cameraIsFreeForm && isCameraFreeForm)
	{
		setCurrentCameraView(Camera::UserDefinedView);
		if (showMessage)
			printTimedUnique("Camera::setIsCameraFreeForm", 7, QT_TR_NOOP("Move the camera by using scroll wheel, double-click to focus or Ctrl + Shift W, A, S, D."));
	}
	cameraIsFreeForm = isCameraFreeForm;
}

// This returns the camera eye when invoking ZoomToAngledBounds
// This method is used for error checking in zooming. This
// makes sure we don't fly out of the desktop box
Vec3 Camera::getDefaultEye()
{
	// Find out the region where this camera will look at
	Bounds bounds;
	for (int i = 0; i < 4; i++)
	{
		Vec3 wallsPos = GLOBAL(WallsPos)[i];
		wallsPos.y = 0;
		bounds.include(wallsPos);
	}

	// Move the camera into position so it encapsulates the entire work area
	Vec3 oldEye = getEye(), oldUp = getUp(), oldDir = getDir();
	Vec3 newEye, newDir, extents;
	bounds.getExtents(extents);
	float distZ = extents.z * 0.70f;
	float distX = extents.x * 0.70f;

	// Move the camera to look at that location
	Vec3 camView;
	camView.x = bounds.getMin().x + ((bounds.getMax().x - bounds.getMin().x) / 2);
	camView.z = bounds.getMin().z + ((bounds.getMax().z - bounds.getMin().z) / 2);

	float aspect = float(winOS->GetWindowWidth()) / float(winOS->GetWindowHeight());
	float hFoV = bounds.getMax().z - bounds.getMin().z;
	float wFoV = bounds.getMax().x - bounds.getMin().x;
	float FoV = CAMERA_FOVY / 2; // 60 / 2 Deg on Y-axis
	float tanFoV = tan(FoV * (PI / 180));
	float multiplier;

	// Fix the multiplier based on the aspect ratio, if needed (aspect ratio only if width is larger)
	if (wFoV / hFoV > aspect)
	{
		// Width is longer then the screen, use it
		multiplier = wFoV / aspect;
	}else{
		// height is longer then the screen
		multiplier = hFoV;
	}

	// distance form the group
	camView.y = bounds.getMax().y + ((multiplier / 2) / tanFoV);

	return camView + Vec3(0, 10, -distZ);
}

Camera::PredefinedCameraViews Camera::getCurrentCameraView()
{
	return _currentCameraView;
}

void Camera::setCurrentCameraView(PredefinedCameraViews view, bool makePersistent)
{
	_currentCameraView = view;
		
	if (view == Camera::TopDownView)
	{
		zoomToTopDownView();	
		if (makePersistent)
			GLOBAL(settings).cameraPreset = QT_NT("oh");
	}
	else if (view == Camera::BottomRightCornerView)
	{
		zoomToBottomRightCornerView();		
		if (makePersistent)
			GLOBAL(settings).cameraPreset = QT_NT("brc");
	}
	else if (view == Camera::UserDefinedView)
	{
		// TODO: We should add code to zoom to the currently
		// user defined camera position. We do not store such 
		// information in settings yet, so just return.
		return;
	}
	else
	{
		zoomToAngledView();		
		if (makePersistent)
			GLOBAL(settings).cameraPreset = QT_NT("def");
	}
	if (makePersistent)
	 	winOS->SaveSettingsFile();
}

void Camera::setOrientation(Vec3 Eye, Vec3 Up, Vec3 Dir)
{
	// Set the orientation of the camera
	setEye(Eye);
	setUp(Up);
	setDir(Dir);
}

void Camera::unserialize(unsigned char *buf, uint &bufSz, int versionNumber)
{
	Vec3 eye, dir, up;
	if (versionNumber >= 11 && 
		SERIALIZE_READ_VEC3(&buf, eye, bufSz) && 
		SERIALIZE_READ_VEC3(&buf, dir, bufSz) && 
		SERIALIZE_READ_VEC3(&buf, up, bufSz))
	{
		savedEye = eye;
		savedDir = dir;
		savedUp = up;
	}
}

void Camera::update(float elapsed)
{
	// if we are watching a set of actors, check their bounds to 
	// see if we need to update the camera
	if (!watchedActors.empty())
	{
		WatchedObjects& watched = watchedActors.top();
		if (!watched.highlightedActor) 
		{
			// check the watched actors' bounds
			Bounds bounds = getActorsBounds(watched.actors, true);				

			// ensure that we are not already zoomed to bounds
			float minMag = (bounds.getMin() - watched.bounds.getMin()).magnitudeSquared();
			float maxMag = (bounds.getMax() - watched.bounds.getMax()).magnitudeSquared();
			if (minMag > 0.005f || maxMag > 0.005f)
			{
				// zoom to bounds
				if (!GLOBAL(isBumpPhotoMode))
				{
					if (isTrackingWatchedActors())
					{
						zoomToBounds(bounds, false);
					}
					watched.bounds = bounds; 		
				}
				rndrManager->invalidateRenderer();
			}
		}
	}
	
	// Check whether or not the ZoomOutControl needs to be rendered.
	if ((eye - orig).magnitude() < SAME_POSITION_THRESHOLD)
	{
		// Hide the ZoomOut control when the camera is at the origin
		if (zoomOutControl.isEnabled())
			zoomOutControl.hide();
	}
	else
	{
		if (isSlideshowModeActive() || GLOBAL(isInSharingMode) || GLOBAL(isInInfiniteDesktopMode))
		{
			// Hide the ZoomOut control when we're in slideshow or sharing mode
			zoomOutControl.hide();
		}
		else if (pathEye.size() > 0 && (pathEye.back() - orig).magnitude() < SAME_POSITION_THRESHOLD)
		{
			// If the camera is animating towards the origin, hide the ZoomOut control
			zoomOutControl.hide();
		}
		else
		{
			// Otherwise, show the zoomOut control
			zoomOutControl.show();
		}
	}
#ifndef DXRENDER
	if (_isglDataDirty)
		readOGLMatrices();
#endif
}

// Bounds checking and adjusting
bool Camera::adjustPointInDesktop(Vec3& origin, Vec3& destination, NxPlane& plane)
{
	Vec3 direction = destination - origin;
	direction.normalize();
	Ray ray(origin, direction);
	Box desktopBox = GetDesktopBox(GLOBAL(ZoomBuffer));
	NxPlane planes[6];
	desktopBox.computePlanes(planes);
	
	// For some reason, intersection will only work correctly if the normals of all the planes face to the inside of the desktop.
	// computePlanes() gives us these two planes (floor and ceiling) with the normals facing outwards.
	planes[2].normal *= -1.0f;
	planes[3].normal *= -1.0f;

	// Two hacks that help keep the rest of BumpTop looking as pretty as it used to be.
	// The first sets the y bound as the default camera position.
	// The second sets the floor y bound to be actually on the floor and not below it.
	planes[2].d = -getDefaultEye().y;
	planes[3].d = 5.0f;

	NxReal xMin, xMax, yMin, yMax, zMin, zMax, distance;
	xMin = desktopBox.GetCenter().x - desktopBox.GetExtents().x;
	xMax = desktopBox.GetCenter().x + desktopBox.GetExtents().x;
	yMin = desktopBox.GetCenter().y - desktopBox.GetExtents().y;
	yMax = getDefaultEye().y;									// desktopBox.GetCenter().y + desktopBox.GetExtents().y;
	zMin = desktopBox.GetCenter().z - desktopBox.GetExtents().z;
	zMax = desktopBox.GetCenter().z + desktopBox.GetExtents().z;
	
	int index = -1;
	int count = 0;
	do 
	{
		if (destination.x > xMax)
		{
			NxRayPlaneIntersect(ray, planes[1], distance, destination);
			destination.x = xMax;
			index = 1;
		}
		else if (destination.x < xMin)
		{
			NxRayPlaneIntersect(ray, planes[0], distance, destination);
			destination.x = xMin;
			index = 0;
		}

		if (destination.y > yMax)
		{
			NxRayPlaneIntersect(ray, planes[2], distance, destination);
			destination.y = yMax;
			index = 2;
		}
		else if (destination.y < yMin)
		{
			NxRayPlaneIntersect(ray, planes[3], distance, destination);
			destination.y = yMin;
			index = 3;
		}

		if (destination.z > zMax)
		{
			NxRayPlaneIntersect(ray, planes[5], distance, destination);
			destination.z = zMax;
			index = 5;
		}
		else if (destination.z < zMin)
		{
			NxRayPlaneIntersect(ray, planes[4], distance, destination);
			destination.z = zMin;
			index = 4;
		}
		count++;
	} while (index != -1 && count < 2);
	
	if (index == -1)
		return false;

	if (count == 2)
	{
		// This means we are stuck in such a position where the origin and
		// destination form a ray that completely exists outside of the 
		// desktop box. We will need to handle this differently.
		// Project the point to the wall it intersected and use that new point.
		ray = Ray(destination, planes[index].normal);
		NxRayPlaneIntersect(ray, planes[index], distance, destination);
	}

	plane = planes[index];
	return true;
}

// eyeDest: the position in world space of the destination the camera eye is to animate to
// dirDest: the direction in world space of the destination the camera direction is to animate to
// upDest: the up direction in world space of the destination the camera up direction is to animate to
// timeStep: the amount of steps the animation should take to get to its destination
// easeIn: whether or not the animation should start slow and speed into the animation
// boundsCheck: whether or not to preform boundary checking
// projectTail: whether or not to preform a projection of the tail end of the vector(made from the current eye position
//				and the eye destination) onto the plane it intersects. boundsCheck must be set to true for this to work. 
void Camera::animateToImpl(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest, int timeStep, bool easeIn, bool boundsCheck, bool projectTail)
{
	EaseFunctionType easeType;
	if (easeIn)
		easeType = Ease;
	else
		easeType = NoEase;

	int originalTimeStep = timeStep;

	dirDest.normalize();
	upDest.normalize();

	deque<Vec3> projectionPath;

	if (boundsCheck)
	{
		// Perform bounds-checking/adjusting
		Vec3 newEyeDest = eyeDest;
		NxPlane plane;
		
		// If the destination is outside the desktop, move it in, and only proceed in the body 
		// of this if statement if we want to project the tail of the vector.
		if (adjustPointInDesktop(eye, eyeDest, plane) && projectTail)
		{
			// Project the destination point onto the plane the vector intersected and make an animation from
			// the point of intersection to this projected point
			NxReal dist;
			Ray ray(newEyeDest, plane.normal);
			NxRayPlaneIntersect(ray, plane, dist, newEyeDest);
			adjustPointInDesktop(eyeDest, newEyeDest, plane);
			float percentage = eyeDest.distance(newEyeDest) / (eye.distance(eyeDest) + eyeDest.distance(newEyeDest));
			int projectedTimeStep = percentage * timeStep;
			timeStep -= projectedTimeStep;
			if (projectedTimeStep > 0)
				projectionPath = lerpRange(eyeDest, newEyeDest, projectedTimeStep, NoEase);
		}
	}

	if (timeStep > 0)
	{
		// Create an animation path over [timeStep] frames
		pathEye = lerpRange(eye, eyeDest, timeStep, easeType);
	}
	
	pathDir = lerpRange(dir, dirDest, originalTimeStep, easeType);
	pathUp = lerpRange(up, upDest, originalTimeStep, easeType);

	// Append, if necessary, the projected animation path to the normal path
	for (int n = 0; n < projectionPath.size(); n++)
	{
		pathEye.push_back(projectionPath[n]);
	}

	animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) ReshapeTextAfterAnim, this));
}

void Camera::animateTo(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest, int timeStep, bool easeIn)
{
	animateToImpl(eyeDest, dirDest, upDest, timeStep, easeIn, false, false);
}

void Camera::animateToWithBounds(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest, int timeStep, bool easeIn)
{
	animateToImpl(eyeDest, dirDest, upDest, timeStep, easeIn, true, false);
}

void Camera::animateToWithSliding(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest, int timeStep, bool easeIn)
{
	animateToImpl(eyeDest, dirDest, upDest, timeStep, easeIn, true, true);
}

void Camera::animateQuadratic(Vec3 eyeDest, Vec3 topDest, Vec3 dirDest, Vec3 upDest, int timeStep)
{
	dirDest.normalize();
	upDest.normalize();


	// Create an animation path over [timeStep] frames
	pathEye = lerpQuardaticCurve(eye, topDest, eyeDest, timeStep);

	// pathEye = lerpQuardaticCurve(eye, eye + (eyeDest-eye)*1.1, eyeDest, timeStep); //quadratic overshoot animation, doesn't look that great
	pathDir = lerpRange(dir, dirDest, timeStep);
	pathUp = lerpRange(up, upDest, timeStep);

	animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) ReshapeTextAfterAnim, NULL));
}

void Camera::customAnimation(deque<Vec3> &eyeDest, deque<Vec3> &dirDest, deque<Vec3> &upDest)
{
	// Set the new custom Path
	pathEye = eyeDest;
	pathDir = dirDest;
	pathUp = upDest;

	animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) ReshapeTextAfterAnim, NULL));
}

void Camera::setAnimationFinishCallback(void (* animationFinishCallback)(void *), void * animationFinishData)
{
	_animationFinishCallback = animationFinishCallback;
	_animationFinishData = animationFinishData;
}

void Camera::animationFinishCallback()
{
	if (_animationFinishCallback)
		_animationFinishCallback(_animationFinishData);
	_animationFinishData = NULL;
	_animationFinishCallback = NULL;
}

//Jumps immediately to where the camera is currently animating too.  
//Note: Leaves 1 frame of animation yet to go.  Since clearing this causes a weird novodex bug (see below)
void Camera::skipAnimation()
{
	animManager->finishAnimation(this);
	/*
	if (pathEye.size() > 0)
	{
		// Get the very last animation frame coordinates
		setEye(pathEye.back());
		setUp(pathUp.back());
		setDir(pathDir.back());

		//Clear all but the last frame of the animation path
		pathEye.erase(pathEye.begin(), pathEye.end() - 1);
		pathUp.erase(pathUp.begin(), pathUp.end() - 1);
		pathDir.erase(pathDir.begin(), pathDir.end() - 1);

	}
	
	//This cleaner version causes a weird bug.  On second drop of icons, icons fall through the floor and keep falling.  
	// Clear the animation paths
	//pathEye.clear();
	//pathUp.clear();
	//pathDir.clear();
	*/
}

void Camera::scrollToPoint(Vec3 loc)
{	
	// Scroll to point doesn't make much sense in infinite desktop mode
	// so if a user tries to zoom in or move around we ignore those requests
	if (GLOBAL(isInInfiniteDesktopMode))
		return;

	// Determine how far the camera is from the point we are moving to
	int facingWall = cameraIsFacingWall();
	Vec3 camWallOrigin = Vec3(0, 75, 0);

	// update the visible bounds if necessary
	updateWatchedActorVisibleBounds();
}

void Camera::scrollToOrigin()
{
	if (GLOBAL(isInInfiniteDesktopMode))
	{
		if (isWatchingActors())
		{
			Vec3 center = getEyePositionToFitBounds(watchedActors.top().bounds);
			orig = center;
		}
	}
	// Scroll to the origin
	if (orig.distance(getEye()) < 125.0f)
		animateTo(orig, getDir(), getUp());

	scrollToPoint(orig);
	
	// dismiss the free-form camera message
	dismiss("Camera::setIsCameraFreeForm");
}

void Camera::setOrigin(Vec3 newOrig)
{
	orig = newOrig;
#ifndef DXRENDER
	_isglDataDirty = true;
#endif
}

Vec3 Camera::getOrigin()
{
	return orig;
}

void Camera::setSlideshow(bool active) {

	isSlideShow = active;

}

bool Camera::inSlideshow() {

	return isSlideShow;

}

void Camera::helperSetThumbnailDetail(BumpObject* actor, GLTextureDetail textureDetail, GLTextureLoadPriority priority)
{
	if (hasWatchedActorHighlighted())
	{
		assert(isWatchedActorHighlighted(actor));
		if (actor->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor *fsActor = (FileSystemActor *)actor; 
			bool drawBorder = true;

			if (fsActor->isFileSystemType(PhotoFrame))
			{
				PhotoFrameActor* pfActor = (PhotoFrameActor*)fsActor;
				if (!pfActor->isSourceType(LocalImageSource))
					return;
				drawBorder = false;
			}

			if (fsActor->isThumbnailized() && !fsActor->hasAnimatedTexture())
			{
				QString texId = fsActor->getThumbnailID();
				QString texPath = fsActor->getTargetPath();
				bool isTrainingImage = texPath.startsWith(native(winOS->GetTrainingDirectory()), Qt::CaseInsensitive);
				
				// NOTE: workaround for ensuring that training images are always hi-resolution
				if (isTrainingImage)
				{
					dismiss("Camera::LoadingHiRes");
				}
				else
				{
					if (!texMgr->hasTextureDetail(texId, textureDetail))
					{
						// NOTE: we do not free the image data since that will happen when the thumbnail replaces this 
						// image anyways (it takes time and this is a blocking call!)
						if (texMgr->loadBlockingTexture(GLTextureObject(Load|Reload, texId, texPath, textureDetail, priority, true, drawBorder), (textureDetail != HiResImage), false))
						{
							rndrManager->invalidateRenderer();
							dismiss("Camera::LoadingHiRes");
						}
						else
						{
							printUniqueError("Camera::LoadingHiRes", QT_TR_NOOP("Could not load the full image!"));
						}
					}
				}
			}
		}
	}
}




void Camera::watchActor( BumpObject * object )
{
	if (!watchedActors.empty())
	{
		WatchedObjects& watched = watchedActors.top();
		vector<BumpObject *>::iterator iter = 
			std::find(watched.actors.begin(), watched.actors.end(), object);
		if (iter == watched.actors.end())
		{
			watched.actors.push_back(object);
		}
	}
}

void Camera::unwatchActor( BumpObject * object )
{
	if (!watchedActors.empty())
	{
		WatchedObjects& watched = watchedActors.top();
		vector<BumpObject *>::iterator iter = 
			std::find(watched.actors.begin(), watched.actors.end(), object);
		if (iter != watched.actors.end())
		{
			watched.actors.erase(iter);
		}
	}
}

void Camera::pushWatchActors( const vector<BumpObject *>& actors, bool groupPileItems)
{
	// check if we are watching the exact actors aready
	if (!watchedActors.empty())
	{
		WatchedObjects& watched = watchedActors.top();
		if (watched.actors.size() == actors.size())
		{
			// check each actor if they are the same
			bool identicalActors = true;
			for (int i = 0; i < actors.size(); ++i)
			{
				if (watched.actors[i] != actors[i])
				{
					identicalActors = false;
					break;
				}
			}

			// if the same, return
			if (identicalActors)
				return;
		}

		// fade in existing items first
		// watchedActors.top().fadeInAll();
	}

	if (!actors.empty())
	{

		WatchedObjects wo(actors);
		orderSpatially2D(wo.actors, groupPileItems);
		wo.storeCamera();
		watchedActors.push(wo);
	}
}

void Camera::popWatchActors()
{
	if (!_onPopWatchActorsHandler.empty())
		_onPopWatchActorsHandler();

	if (!watchedActors.empty())
	{
		if (watchedActors.top().highlightedActor)
		{	
			if (scnManager->containsObject(watchedActors.top().highlightedActor))
			{
				dismiss("Camera::LoadingHiRes");
				assert(watchedActors.top().highlightedActor->getObjectType() == ObjectType(BumpActor, FileSystem));
				FileSystemActor * fsActor = (FileSystemActor *) watchedActors.top().highlightedActor;
				fsActor->setMinThumbnailDetail(SampledImage);
				helperSetThumbnailDetail(watchedActors.top().highlightedActor, SampledImage, LowPriority);
			}
			else
			{
				// erase the highlight actor from the set to have their nameables restored
				_prevVisibleNameables.erase(watchedActors.top().highlightedActor);
			}
		}

		// fade in items first
		// watchedActors.top().fadeInAll();

		// XXX: HACKY way to get around not reseting the view angled 
		//		`on no objects with walls down
		WatchedObjects wo(watchedActors.top());
		wo.restoreCamera();
		if (watchedActors.size() > 1 || GLOBAL(DrawWalls))
		{
			watchedActors.pop();
		}
		else if (!watchedActors.empty())
		{
			watchedActors.top().highlightedActor = NULL;
		}

		// finish logging slideshow time if we were in slideshow at all
		statsManager->finishTimer(statsManager->getStats().bt.window.slideshowTime);

		// dismiss the slideshow message if there is one
		dismiss("Key_ToggleSlideShow");
	}

	// fade in the slideshow controls
	slideShowControls.disable();
}





void Camera::popAllWatchActors()
{
	while (!watchedActors.empty())
	{
		if (watchedActors.top().highlightedActor)
		{
			if (scnManager->containsObject(watchedActors.top().highlightedActor))
			{
				dismiss("Camera::LoadingHiRes");
				assert(watchedActors.top().highlightedActor->getObjectType() == ObjectType(BumpActor, FileSystem));
				FileSystemActor * fsActor = (FileSystemActor *) watchedActors.top().highlightedActor;
				fsActor->setMinThumbnailDetail(SampledImage);
				helperSetThumbnailDetail(watchedActors.top().highlightedActor, SampledImage, LowPriority);
			}
			else
			{
				// erase the highlight actor from the set to have their nameables restored
				_prevVisibleNameables.erase(watchedActors.top().highlightedActor);
			}
		}

		watchedActors.pop();
	}

	// dismiss the slideshow message if there is one
	dismiss("Key_ToggleSlideShow");
}

void * LoadNextHighResThumbnailAfterAnim(AnimationEntry *animEntry)
{
	BumpObject * actor = (BumpObject *) animEntry->getCustomData();
	if (scnManager->containsObject(actor))
	{
		assert(actor->getObjectType() == ObjectType(BumpActor, FileSystem));
		FileSystemActor * fsActor = (FileSystemActor *) actor;
		fsActor->setMinThumbnailDetail(HiResImage);
		cam->helperSetThumbnailDetail(actor, HiResImage, LowPriority);
	}
	return NULL;
}

void * LoadHighResImageAfterAnim(AnimationEntry *animEntry)
{
	BumpObject * actor = (BumpObject *) animEntry->getCustomData();
	if (scnManager->containsObject(actor))
	{
		assert(actor->getObjectType() == ObjectType(BumpActor, FileSystem));
		FileSystemActor * fsActor = (FileSystemActor *) actor;
		fsActor->setMinThumbnailDetail(HiResImage);
		cam->helperSetThumbnailDetail(actor, HiResImage, LowPriority);

		/*
		BumpObject * nextObj = cam->getNextHighlightWatchedActor(true);
		nextObj->finishAnimation();
		nextObj->setPoseAnim(nextObj->getGlobalPose(), nextObj->getGlobalPose(), 50);
		animManager->addAnimation(AnimationEntry(cam, (FinishedCallBack) LoadNextHighResThumbnailAfterAnim, (void *) nextObj));
		*/
	}
	return NULL;
}


void * SetupActorForSlideshow(AnimationEntry *animEntry)
{
	if (!isSlideshowModeActive())
		return NULL;
	Vec3 newEye, newUp, newDir;
	cam->zoomToOrientedActorBoundsAttributes(cam->getHighlightedWatchedActor(), newEye, newUp, newDir);
	cam->setEye(newEye);
	cam->setUp(newUp);
	cam->setDir(newDir);
	LoadHighResImageAfterAnim(animEntry);
	return NULL;
}


void Camera::rehighlightWatchedActor()
{
	// ensure items are being watched
	if (watchedActors.empty())
		return;

	WatchedObjects& watched = watchedActors.top();
	if (!watched.highlightedActor)
		return;

	// clear existing animation
	pathEye.clear();
	pathDir.clear();
	pathUp.clear();

	// get the new camera attributes
	Vec3 newEye, newUp, newDir;
	zoomToOrientedActorBoundsAttributes(watched.highlightedActor, newEye, newUp, newDir);

	// add animation to get the movement from the previous to new actor
	const int timeStep = watched.highlightedActor ? 85 : 75;
	animateTo(newEye, newDir, newUp, timeStep);
}

void Camera::highlightWatchedActor( BumpObject * actor, BumpObject * nextActor )
{
	static BumpObject * prevActor = NULL;
	static bool prevActorWasOn = true;

	// re-enable previously disabled photo frames
	if (prevActor && prevActor->isObjectType(ObjectType(BumpActor, FileSystem, PhotoFrame)))
	{
		// When the slideshow is being ended, this function will be called with NULL,NULL. We check for this here because in certain cases
		// prevActor can be a dangling pointer (zoom in on photoframe and hit delete key)
		if (prevActorWasOn) 
		{			
			if (!dynamic_cast<PhotoFrameActor *>(prevActor)->isUpdating())
				dynamic_cast<PhotoFrameActor *>(prevActor)->enableUpdate();
		}
	}
	// disable photo frames from updating while 
	if (actor && actor->isObjectType(ObjectType(BumpActor, FileSystem, PhotoFrame)))
	{
		//Current actor is a photoframe
		prevActorWasOn = dynamic_cast<PhotoFrameActor *>(actor)->isUpdating();
		prevActor = actor;

		if (dynamic_cast<PhotoFrameActor *>(actor)->isUpdating())
			dynamic_cast<PhotoFrameActor *>(actor)->disableUpdate();
	}

	// ensure items are being watched
	if (watchedActors.empty())
		return;

	WatchedObjects& watched = watchedActors.top();

	// ensure different actor
	if (actor == watched.highlightedActor)
		return;

	// ensure that the actor is currently being watched
	vector<BumpObject *>::const_iterator actorIter = 
		find(watched.actors.begin(), watched.actors.end(), actor);
	if (actorIter == watched.actors.end())
		return;

	// init the camera controls if not already
	slideShowControls.init();

	// get the eye position for both the previous and new actor (top of curve)
	Vec3 bothActorsEye(eye);
	bool bothActorsPinned = false;
	if (watched.highlightedActor)
	{
		// animate to zoom to fit the previous and the new actor
		Bounds watchedBounds;
		watchedBounds.combine(watched.highlightedActor->getBoundingBox());
		watchedBounds.combine(actor->getBoundingBox());
		bothActorsEye = getEyePositionToFitBounds(watchedBounds);
		bothActorsPinned = watched.highlightedActor->isPinned() && actor->isPinned();
	}
	
	// clear existing animation
	pathEye.clear();
	pathDir.clear();
	pathUp.clear();

	// get the new camera attributes
	Vec3 newEye, newUp, newDir;
	zoomToOrientedActorBoundsAttributes(actor, newEye, newUp, newDir);


	// determine if we should lerp or slerp the eye (in any other context...)
	// we do this by checking if the angles between the new and old up vecs differ too greatly,
	// or if both items are pinned
	bool lerp = bothActorsPinned;
	float theta = acos(up.dot(newUp) / (up.magnitude() * newUp.magnitude()));
	float maxAngleDiff = PI / 4.0f;		// 45 degrees
	if (theta >= maxAngleDiff)
	{
		lerp = true;
	}
	
	// add animation to get the movement from the previous to new actor
	const int timeStep = watched.highlightedActor ? 85 : 75;
	if (lerp || !watched.highlightedActor)
		pathEye = lerpRange(eye, newEye, timeStep);
	else
	{
		pathEye = lerpQuardaticFittingCurve(eye, bothActorsEye, newEye, timeStep, SoftEase);
	}
	pathDir = lerpRange(dir, newDir, timeStep, SoftEase);
	pathUp = lerpRange(up, newUp, timeStep, SoftEase);

	// update the highlighted actor
	// (we must do this before we fade out the other actors)
	watched.highlightedActor = actor;
	
	// if it's a filesystem actor
	if (!(actor->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame)))
		printUnique("Camera::LoadingHiRes", QT_TR_NOOP("Loading full image"));
	{
		deque<Vec3> eyePath = pathEye;
		deque<Vec3> dirPath = pathDir;
		deque<Vec3> upPath = pathUp;
		animManager->removeAnimation(this);
		pathEye = eyePath;
		pathDir = dirPath;
		pathUp = upPath;
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) SetupActorForSlideshow, (void *) watched.highlightedActor));
	}

	// fade in the highlighted actor
	// watched.fadeOutAllButHighlighted(timeStep);

	// fade out all the nameables
	// NOTE: we only do this for the first highlight, which is why we test if 
	//		 the next actor is NULL.  
	if (!nextActor)
	{
		storePreviousVisibleNameables(true);
	}

	if (actor->getObjectType() == ObjectType(BumpActor, FileSystem, Image) &&
		actor->getObjectType() != ObjectType(BumpActor, FileSystem, PhotoFrame))
	{
		FileSystemActor* photo = dynamic_cast<FileSystemActor*>(actor);
		if (hasOriginalPhoto(photo))
		{
			getRestoreOriginalPhotoControl()->show();
		}	
		else
		{
			getRestoreOriginalPhotoControl()->hide();
		}
	}
	else
	{
		getRestoreOriginalPhotoControl()->hide();
	}
}

int Camera::cameraIsFacingWall()
{
	// Check if camera is pointing to a wall
	// Construct ray going through the camera eye and camera direction
	Ray camRay(getEye(), getDir());
	tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(camRay);
	int wallIndex = t.get<0>();
	return wallIndex;
}

void Camera::lookAtWall(NxActorWrapper* wall, Vec3& positionOut, Vec3& directionOut)
{
	assert(wall);
	positionOut = Vec3(0, 75, 0);
	directionOut = wall->getGlobalPosition();

	directionOut.y = 20;
	directionOut -= positionOut;
	directionOut.normalize();

	// Move the camera back to get the entire wall
	positionOut += -(directionOut * 100);
	adjustPointToInsideWalls(positionOut);

	_currentCameraView = WallView;
}

bool Camera::isWatchingActors() const
{
	return !watchedActors.empty();
}

bool Camera::isTrackingWatchedActors() const
{
	return isWatchingActors() && watchedActors.top().trackActors;
}

void Camera::setTrackingWatchedActors(bool state)
{
	if (isTrackingWatchedActors())
	{
		watchedActors.top().trackActors = state;
	}
}

bool Camera::isWatchedActorHighlighted( BumpObject * object ) const
{
	return (!watchedActors.empty() && watchedActors.top().highlightedActor == object);
}

bool Camera::hasWatchedActorHighlighted() const
{
	return (!watchedActors.empty() && watchedActors.top().highlightedActor);
}

BumpObject * Camera::getHighlightedWatchedActor() const
{
	if (hasWatchedActorHighlighted())
	{
		return watchedActors.top().highlightedActor;
	}
	return NULL;
}

void Camera::highlightNextWatchedActor(bool forward)
{

	if (!_onHighlightNextWatchedActorHandler.empty())
		_onHighlightNextWatchedActorHandler(forward);

	// ensure items are being watched
	if (watchedActors.empty())
		return;

	WatchedObjects& watched = watchedActors.top();

	// ensure there is an actor being highlighted
	if (!watched.highlightedActor)
		return;
	
	// ensure that there are other items to highlight
	int num = watched.actors.size();
	if (num <= 1)
		return;

	// find it in the watched list
	for (int i = 0; i < num; ++i)
	{
		if (watched.actors[i] == watched.highlightedActor)
		{
			int index = i;
			int nextIndex = i;
			int prevIndex = i;
			if (forward)
			{
				prevIndex = (i+num-1) % num;
				index = (i+1) % num;
				nextIndex = (i+2) % num;
			}
			else
			{
				prevIndex = (i+1) % num;
				index = (i+num-1) % num;
				nextIndex = (i+num-2) % num;
			}

			// pop the high-res image for the currently highlighted actor
			assert(watched.highlightedActor->getObjectType() == ObjectType(BumpActor, FileSystem));
			FileSystemActor * fsActor = (FileSystemActor *) watched.highlightedActor;
			fsActor->setMinThumbnailDetail(SampledImage);
			helperSetThumbnailDetail(watched.highlightedActor, SampledImage, HighPriority);

			// load the next item (and preload the next next item)
			highlightWatchedActor(watched.actors[index], watched.actors[nextIndex]);
			return;
		}
	}
}

Vec3 Camera::getEyePositionToFitBounds(const Bounds& bounds)
{
	// save camera orientation
	Vec3 tempEye(eye);
	Vec3 tempUp(up);
	Vec3 tempDir(dir);
	// calculate new eye position to fit bounds
	Vec3 newEye = zoomToBounds(bounds, false);
	// restore camera orientation
	setEye(tempEye);
	setUp(tempUp);
	setDir(tempDir);

	return newEye;
}

void Camera::zoomToIncludeObjects(vector<BumpObject*> objs)
{
	Bounds bounds = getActorsBounds(objs);
	zoomToBounds(bounds);
}

Vec3 Camera::zoomToTopDownView()
{
	setIsCameraFreeForm(false);

	// calculate how high the camera must be to view the whole desktop
	Box desktopBox = GetDesktopBox();
	float aspect = desktopBox.extents.x / desktopBox.extents.z;
	float invAspect = 1.0f / aspect;
	float xFov = 60.0f / 2.0f;
	float yFov = xFov / aspect;
	int depth = desktopBox.extents.z;
	int height = ((invAspect - 0.022f) * depth) / tan(yFov * (PI / 180));

	Vec3 newEye(0, height, 0);
	setOrigin(newEye);
	animateTo(newEye, Vec3(0, -1, 0), Vec3(0, 0, 1));
	return newEye;
}

Vec3 Camera::zoomToAngledView()
{
	setIsCameraFreeForm(false);

	// calculate how high the camera must be to view the whole desktop
	Box desktopBox = GetDesktopBox();
	float aspect = desktopBox.extents.x / desktopBox.extents.z;
	float invAspect = 1.0f / aspect;
	float xFov = 60.0f / 2.0f;
	float yFov = xFov / aspect;
	int depth = desktopBox.extents.z;
	int height = ((invAspect - 0.022f) * depth) / tan(yFov * (PI / 180));

	Vec3 newEye = Vec3(0, height, 0);
	Vec3 newDir = Vec3(0, -1, 0);
	Vec3 newUp = Vec3(0, 0, 1);

	// get the rotated centroid point relative to the bottom wall edge
	float angle = 18.0f;
	Quat ori(angle, Vec3(-1,0,0));
	ori.rotate(newEye);
	ori.rotate(newDir);
	ori.rotate(newUp);

	// offset to be flush with the bottom wall edge
	Vec3 bottomWallEdgePt(0, 0, -depth);
	Vec3 rotatedBottomWallEdgePt(-newUp * depth);
	Vec3 diff = rotatedBottomWallEdgePt - bottomWallEdgePt;
	newEye -= diff;

	setOrigin(newEye);
	animateTo(newEye, newDir, newUp);
	return newEye;
}

Vec3 Camera::zoomToBottomRightCornerView()
{
	setIsCameraFreeForm(false);

	Vec3 center, extents;
	Vec3 newEye, direction, up;
	Bounds wall;
	vector<NxActorWrapper*> walls = GLOBAL(Walls);
	
	// bottom wall
	wall = walls[1]->getBoundingBox();
	wall.getCenter(center);
	wall.getExtents(extents);

	newEye = center - Vec3(extents.x * 0.8, 0 , 0);
	newEye.z = 0;
	newEye.z = center.z * 0.8;
	setOrigin(newEye);

	// right wall
	wall = walls[2]->getBoundingBox();
	wall.getCenter(center);
	wall.getExtents(extents);

	direction = -newEye;
	direction.x = center.x / -2;
	up.cross(direction, Vec3(0, 1, 0));
	up.cross(up, direction);
	animateTo(newEye, direction, up);

	return newEye;
}

void Camera::zoomToOrientedActorBoundsAttributes(BumpObject * actor, Vec3& eye, Vec3& up, Vec3& dir, float buffer, const Vec3 & screenSize)
{
	// get the values
	up = actor->getGlobalOrientation() * Vec3(0, 1, 0);
	dir = actor->getGlobalOrientation() * Vec3(0, 0, 1);
	up.normalize();
	up.y = abs(up.y);
	dir.normalize();

	// check if the direction of the frame is aligned to the direction to the actor
	// if not, it is backwards, so flip the direction.  we do this in a hacky fashion
	// by checking if the dot of the actor direction and the origin (offsetted high,
	// so that the items on the floor are accounted for) are "opposite".
	Vec3 actorWorldDir = actor->getGlobalPosition();
	actorWorldDir -= Vec3(0, getEye().y, 0);
	if (actorWorldDir.dot(dir) < 0)
		dir.setNegative();

	eye = actor->getGlobalPosition();

	Box box = actor->getBox();
	float width = float(winOS->GetWindowWidth());
	float height = float(winOS->GetWindowHeight());
	float aspect = width / height;
	float hFoV = box.extents.y;
	float wFoV = box.extents.x;

	float zoomHeight = box.extents.y;
	float zoomWidth = box.extents.x;

	Vec3 effectiveScreenSize = Vec3(screenSize.x, screenSize.y, screenSize.z);

	if(isSlideshowModeActive()) {
		effectiveScreenSize.x = width*0.95; //leave a bit of buffer
		effectiveScreenSize.y = height*0.95 - slideShowControls.getHeight();
	}

	if (!effectiveScreenSize.isZero())
	{
#ifdef DXRENDER
		wFoV *= dxr->viewport.Width / effectiveScreenSize.x;
		hFoV *= dxr->viewport.Height / effectiveScreenSize.y;
#else
		wFoV *= _glViewport[2] / effectiveScreenSize.x;
		hFoV *= _glViewport[3] / effectiveScreenSize.y;
#endif
	}

	float FoV = 60.0f / 2; // 60 / 2 Deg on Y-axis
	float tanFoV = tan(FoV * (PI / 180));

	// Fix the multiplier based on the aspect ratio, if needed (aspect ratio only if width is larger)
	float multiplier;
	if (wFoV / hFoV > aspect)
	{
		// Width is longer then the screen, use it
		multiplier = wFoV / aspect;
	}else{
		// height is longer then the screen
		multiplier = hFoV;
	}

	// distance from the actor
	eye -= (dir * (multiplier * NxMath::max(1.1f, buffer) / tanFoV));

	// translate the camera down by the slideshow overlay height if we're in slideshow mode
	if(isSlideshowModeActive()) {
		//The greater of zoomHeight and zoomWidth will in effect represent the height of the screen in bumptop world units
		//if we multiply this value by the ratio of the overlay to the screen, we get the real-world size of the overlay,
		//and we can offset the camera accordingly
		float slideShowOverlayToScreenRatio = slideShowControls.getHeight()/height;
		float offsetDistance = zoomHeight * slideShowOverlayToScreenRatio;
		eye -= (up * offsetDistance);
	}

#ifndef DXRENDER
	_isglDataDirty = true; 
#endif
}

Bounds Camera::getActorsBounds( const vector<BumpObject *>& objects, bool flatten/*=False*/ )
{
	Bounds bounds, aabb;
	Vec3 center, extents;
	float maxY = 0.0f;
	for (uint i = 0; i < objects.size(); i++)
	{
		if (objects[i]->isBumpObjectType(BumpPile) || 
			objects[i]->isBumpObjectType(BumpActor))
		{
			if (!scnManager->containsObject(objects[i]))
				continue;

			// determine the max bounds of the 
			if (objects[i]->isBumpObjectType(BumpPile))
			{
				Pile * p = (Pile *) objects[i];
				if (p->getNumItems() > 0)
				{
					Bounds pileBounds = p->getPileBounds();
					Vec3 pileCenter, pileExtents;
					pileBounds.getCenter(pileCenter);
					pileBounds.getExtents(pileExtents);
					if (p->getPileState() == Grid)
						maxY = NxMath::max(pileCenter.y, maxY);
					else
						maxY = NxMath::max(pileExtents.y, maxY);
				}
			}
			else
			{
				maxY = NxMath::max(objects[i]->getDims().z, maxY);
			}

			aabb = objects[i]->getBoundingBox();
			if (!objects[i]->isPinned() && flatten)
			{
				// flatten the bounds?
				aabb.getCenter(center);
				aabb.getExtents(extents);
				if (!((objects[i]->isBumpObjectType(BumpPile)) && 
					(((Pile *) objects[i])->getPileState() == Grid)))
				{
					center.y = 0;
				}
				extents.y = 0; 
				aabb.setCenterExtents(center, extents);
			}
			bounds.combine(aabb);
		}
	}
	
	if (flatten)
	{
		bounds.getCenter(center);
		bounds.getExtents(extents);
		center.y = extents.y = maxY;
		bounds.setCenterExtents(center, extents);
	}
	return bounds;
}

void Camera::updateWatchedActorVisibleBounds()
{	
	// reset the visible bounds
	visibleBounds.setEmpty();

	// get the view corners in world space (v/w are near/far clipping pane points)
	//
	// 0-------1 <----- v[] and w[] index
	// |       | 
	// 3-------2 
	Vec3 v[4], w[4], pt[4], dir;		
	window2world(0, 0, v[0], w[0]);
	window2world(winOS->GetWindowWidth(), 0, v[1], w[1]);
	window2world(winOS->GetWindowWidth(), winOS->GetWindowHeight(), v[2], w[2]);	
	window2world(0, winOS->GetWindowHeight(), v[3], w[3]);

	// get the floor bounds of the corner points
	float dist;
	Ray r;
	NxPlane floor = NxPlane(Vec3(0,0,0), Vec3(0,1,0));
	for (int i = 0; i < 4; i++)
	{
		// Project this ray to the floor
		dir = w[i]-v[i];
		dir.normalize();
		r = Ray(v[i], dir);
		NxRayPlaneIntersect(r, floor, dist, pt[i]);
	}

	// when in bounded mode, use the bottom points X as the top points X
	if (!GLOBAL(isInInfiniteDesktopMode))
	{
		pt[0].x = pt[3].x;
		pt[1].x = pt[2].x;
	}

	for (int i = 0; i < 4; ++i)
	{		
		// add the point to the bounds
		pt[i].y = 0;
		visibleBounds.include(pt[i]);
	}

}

ZoomOutControl* Camera::getZoomOutControl()
{
	return &zoomOutControl;
}

RestoreOriginalPhotoControl* Camera::getRestoreOriginalPhotoControl()
{
	return &restoreOriginalPhotoControl;
}

void Camera::pointCameraTo(Vec3 pt)
{
	pointCameraTo(pt, true);
}

// Reorient the camera to point to a location the user clicked
void Camera::pointCameraTo(Vec3 pt, bool executeWindowToWorld)
{
	if (!GLOBAL(isInInfiniteDesktopMode))// || isCameraFreeForm() || GLOBAL(settings).enableDebugKeys)
	{
		Vec3 pointOnPlane = pt;

		// Determine what point in the bumptop desktop the mouse has clicked
		if (executeWindowToWorld)
		{
			Vec3 closePoint, farPoint, dir;
			window2world(pt.x, pt.y, closePoint, farPoint);
			dir = farPoint - closePoint;
			dir.normalize();
			Ray theRay = Ray(closePoint, dir);

			// Get unbounded box and planes making up the desktop
			tuple <int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(theRay);
			pointOnPlane = t.get<2>();
		}

		// Make sure the point is inside the bumptop desktop
		Box desktopBox = GetDesktopBox(0);
		if (desktopBox.GetExtents().y + desktopBox.GetCenter().y < pointOnPlane.y)
		{
			return;
		}

		// Determine new camera eye
		Vec3 newEye = getEye();
		if (getEye().distanceSquared(getDefaultEye()) < 0.005)
			newEye.y = GLOBAL(WallHeight) * 0.8;

		//Determine the new direction based on the point on the plane
		Vec3 newDir;
		newDir =  pointOnPlane - newEye;
		newDir.normalize();

		//Determine new up vector
		Vec3 newUp;
		newUp.cross(newDir, Vec3(0,1,0));
		newUp.cross(newUp, newDir);

		// Animate to new direction
		animateTo(newEye, newDir, newUp);		

		storeCameraPosition(newEye, newDir, newUp);
	}
	else
	{
		
	}
}

// This method is used to load a camera preset. 
// It is called when bumptop is initialized or whenever the 
// bumptop settings are modified
void Camera::loadCameraFromPreset(QString cameraPreset)
{
	// get the new preset
	PredefinedCameraViews newCameraPreset = Camera::DefaultView;
	if (cameraPreset.startsWith("oh"))
		newCameraPreset = Camera::TopDownView;
	else if (cameraPreset.startsWith(QT_NT("brc")))
		newCameraPreset = Camera::BottomRightCornerView;

	// don't do anything if the walls haven't been created yet?
	vector<NxActorWrapper*> walls = GLOBAL(Walls);
	assert(!walls.empty());
	if (walls.empty())
		return;

	// if we are zoomed in on any object, we should zoom back out first
	popAllWatchActors();
	WebActor::zoomOutFocusedWebActor();

	// apply the camera change	
	setCurrentCameraView(newCameraPreset);
}

void Camera::onAnimTick()
{
	// Get the next Step
	if (!pathEye.empty())
	{
		setEye(pathEye.front());
		pathEye.pop_front();
	}
	if (!pathDir.empty())
	{
		setDir(pathDir.front());
		pathDir.pop_front();
	}
	if (!pathUp.empty())
	{
		setUp(pathUp.front());
		pathUp.pop_front();
	}

	// Force the camera to be upright, always
	if (up.y < 0.0) up.y = 0.0;

	// Calculate the Right vector
	right.cross(up, dir);
}

void Camera::onAnimFinished()
{
#ifndef DXRENDER
	// NOTE: we update the OGL matrices so that picking works and everything 
	//		 after the camera view has moved
	_isglDataDirty = true;
	readOGLMatrices();
#endif

	// restore the visibility of the previously hidden nameables
	if (!hasWatchedActorHighlighted() && 
		!(getZoomedObjects().size() == 1 && getZoomedObjects().front()->getObjectType() == ObjectType(BumpActor,Webpage)))
		restorePreviousVisibleNameables(false);

	animationFinishCallback();
}

void Camera::killAnimation()
{
	pathEye.clear();
	pathDir.clear();
	pathUp.clear();


	// Remove ourself from the animation queue
	animManager->removeAnimation(this);

	setAnimationState(NoAnim);
}

void Camera::finishAnimation()
{
	// Jump to the end of each animation
	if (!pathEye.empty()) setEye(pathEye.back());
	if (!pathDir.empty()) setDir(pathDir.back());
	if (!pathUp.empty()) setUp(pathUp.back());

	pathEye.clear();
	pathDir.clear();
	pathUp.clear();
	animationFinishCallback();
}

void Camera::revertAnimation()
{
	// Jump to the beginning of each animation
	if (!pathEye.empty()) setEye(pathEye.front());
	if (!pathDir.empty()) setDir(pathDir.front());
	if (!pathUp.empty()) setUp(pathUp.front());

	killAnimation();
}

bool Camera::isAnimating( uint animType /*= SizeAnim | AlphaAnim | PoseAnim*/ )
{
	bool animRunning = false;

	// Check the different types of anims
	if (animType & PoseAnim && (!pathEye.empty() || !pathDir.empty() || !pathUp.empty())) animRunning = true;

	// Check animation Paths
	return animRunning;
}

#ifndef DXRENDER
void Camera::readOGLMatrices()
{
	// We shouldn't have to call switchToPerspective here, but
	// it's a workaround so we have the most up-to-date view matrix
	switchToPerspective();
	glGetDoublev (GL_PROJECTION_MATRIX, _glProjMatrix);
	glGetDoublev (GL_MODELVIEW_MATRIX, _glMVMatrix);
	_glViewport[2] = winOS->GetWindowWidth();
	_glViewport[3] = winOS->GetWindowHeight();
	_isglDataDirty = false;
}

GLdouble * Camera::glModelViewMatrix() const
{
	return _glMVMatrix;
}

GLdouble * Camera::glProjectionMatrix() const
{
	return _glProjMatrix;
}

GLint * Camera::glViewport() const
{
	return _glViewport;
}

void Camera::markglDataDirty()
{
	_isglDataDirty = true;
}
#endif

void Camera::setOnHighlightNextWatchedActorHandler( boost::function<void(bool)> onHighlightNextWatchedActorHandler )
{
	_onHighlightNextWatchedActorHandler = onHighlightNextWatchedActorHandler;
}

void Camera::setOnPopWatchActorsHandler( boost::function<void()> onPopWatchActorsHandler )
{
	_onPopWatchActorsHandler = onPopWatchActorsHandler;
}

// Call with temporary to temporarily hide or restore texts according to last 
// call of storePreviousVisibleNameables without temporary flag
void Camera::restorePreviousVisibleNameables(bool skipAnimation, bool temporary)
{
	set<BumpObject *>::iterator iter = _prevVisibleNameables.begin();
	while (iter != _prevVisibleNameables.end())
	{
		if (scnManager->containsObject(*iter))
		{
			if (skipAnimation)
				(*iter)->showText(true);
			else
				(*iter)->showTextAnimated(25);
		}
		iter++;
	}
	if (!temporary)
		_prevVisibleNameables.clear();
}

void Camera::storePreviousVisibleNameables(bool skipAnimation, bool temporary)
{
	if (temporary)
	{
		// hide text for items that were previously hidden before restorePreviousVisibleNameables called with temporary
		set<BumpObject *>::iterator iter = _prevVisibleNameables.begin();
		while (iter != _prevVisibleNameables.end())
		{
			if (scnManager->containsObject(*iter) && !(*iter)->isTextHidden())
				(*iter)->hideText(skipAnimation);
			iter++;
		}
	}
	else
	{
		// restore the existing nameables first just in case
		restorePreviousVisibleNameables(true);
	
		assert(_prevVisibleNameables.empty());
		vector<BumpObject *> objs = scnManager->getBumpObjects();
		for (int i = 0; i < objs.size(); ++i)
		{
			if (!objs[i]->isTextHidden())
			{
				_prevVisibleNameables.insert(objs[i]);
				objs[i]->hideText(skipAnimation);
			}
		}
	}
}

CornerInfoControl * Camera::getCornerInfoControl()
{
	return &cornerInfoControl;
}

QList<BumpObject*>& Camera::getZoomedObjects()
{
	return zoomedObjects;
}

bool Camera::serializeToPb(PbCamera * pbCamera)
{
	assert(pbCamera);

	toPbVec3(getEye(), pbCamera->mutable_eye());
	toPbVec3(getDir(), pbCamera->mutable_dir());
	toPbVec3(getUp(), pbCamera->mutable_up());

	return true;
}

bool Camera::deserializeFromPb(const PbCamera * pbCamera)
{
	assert(pbCamera);

	if (pbCamera->has_eye()) {
		savedEye = fromPbVec3(pbCamera->eye());
	}
	if (pbCamera->has_dir()){
		savedDir = fromPbVec3(pbCamera->dir());
	}
	if (pbCamera->has_up()) {
		savedUp = fromPbVec3(pbCamera->up());
	}
	return true;
}