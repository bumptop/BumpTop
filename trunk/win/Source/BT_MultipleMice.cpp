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
#include "BT_MultipleMice.h"
#include "BT_MouseEventManager.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"

MultipleMice::MultipleMice()
{
	//ShowCursor(false);

	RAWINPUTDEVICE rid={1,2,0,winOS->GetWindowsHandle()};
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

MultipleMice::~MultipleMice()
{
}

// WM_INPUT
void MultipleMice::onRawInput(WPARAM wParam, LPARAM lParam)
{
	RAWINPUT ri;
	UINT cbri=sizeof(ri);
	GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &ri, &cbri, sizeof(ri.header));
	
	if(ri.header.hDevice!=NULL && ri.header.dwType == RIM_TYPEMOUSE)
	{
		
		if (hDevice_to_mousePointer.find(ri.header.hDevice) == hDevice_to_mousePointer.end())
		{
			hDevice_to_mousePointer[ri.header.hDevice] = new MousePointer();
		}
		
		MousePointer *pointer = hDevice_to_mousePointer[ri.header.hDevice];

		
		UINT message = NULL;
		switch(ri.data.mouse.usButtonFlags)
		{
		case RI_MOUSE_LEFT_BUTTON_DOWN:	
			pointer->setMouseButtons(pointer->getMouseButtons() | MouseButtonLeft);
			message = WM_LBUTTONDOWN; 
			break;
		case RI_MOUSE_LEFT_BUTTON_UP:		
			pointer->setMouseButtons(pointer->getMouseButtons() & ~MouseButtonLeft);
			message = WM_LBUTTONUP; 
			break;
		case RI_MOUSE_MIDDLE_BUTTON_DOWN:	
			pointer->setMouseButtons(pointer->getMouseButtons() | MouseButtonMiddle);
			message = WM_MBUTTONDOWN; 
			break;
		case RI_MOUSE_MIDDLE_BUTTON_UP:	
			pointer->setMouseButtons(pointer->getMouseButtons() & ~MouseButtonMiddle);
			message = WM_MBUTTONUP; 
			break;
		case RI_MOUSE_RIGHT_BUTTON_DOWN:
			pointer->setMouseButtons(pointer->getMouseButtons() | MouseButtonRight);
			message = WM_RBUTTONDOWN; 
			break;
		case RI_MOUSE_RIGHT_BUTTON_UP:	
			pointer->setMouseButtons(pointer->getMouseButtons() & ~MouseButtonRight);
			message = WM_RBUTTONUP; 
			break;
		case RI_MOUSE_WHEEL:
			message = WM_MOUSEWHEEL;
			break;
		default:
			if (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0)
				message = WM_MOUSEMOVE;
			else
				return;
			break;
		}

		pointer->setX(pointer->getX() + ri.data.mouse.lLastX);
		pointer->setY(pointer->getY() + ri.data.mouse.lLastY);

		SetCursorPos(pointer->getX(), pointer->getY());

		POINT pt={pointer->getX(), pointer->getY()};

		// special case: if there is a window on top of our main window, don't handle it here;
		// calling DefWindowProc down the line will send that window the appropriate mouse event
		if (WindowFromPoint(pt) != winOS->GetWindowsHandle()) return;

		ScreenToClient(winOS->GetWindowsHandle(), &pt);

		mouseManager->onMouseEvent(message, 
									pt.x, 
									pt.y, 
									ri.data.mouse.usButtonData, // mouseWheel
									pointer);

	}
}

void MultipleMice::onRender()
{
	if (!hDevice_to_mousePointer.empty())
	{
		/// CURSORS
#ifdef DXRENDER
		// TODO DXR
#else
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, winOS->GetWindowWidth(), 0, winOS->GetWindowHeight());


		glBindTexture(GL_TEXTURE_2D, NULL);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);


		glMatrixMode(GL_MODELVIEW);

		for (hash_map<HANDLE,MousePointer*>::iterator it = hDevice_to_mousePointer.begin();
			it != hDevice_to_mousePointer.end();
			it++)
		{
			renderCursor(it->second);
		}
		

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
#endif
	}
}


void MultipleMice::renderCursor(MousePointer* pointer)
{
		POINT p = {pointer->getX(), pointer->getY()};
		ScreenToClient(winOS->GetWindowsHandle(), &p);

#ifdef DXRENDER
		// TODO DXR
#else
		glLoadIdentity();
		glTranslatef((GLfloat)p.x,(GLfloat)winOS->GetWindowHeight()-p.y,0.0f);
		glScalef(30.0, 30.0, 0.0f);

		glColor4f(1.0f, 0.0f, 0.0f, 0.75f);

		glBegin(GL_TRIANGLES);								// Drawing Using Triangles
		glVertex3f( 0.5f, -0.5f, 0.0f);					// Top

		glVertex3f( 0.0f, 0.0f, 0.0f);					// Bottom Right
		glVertex3f( 0.2f, -0.7f, 0.0f);					// Bottom Left
		glEnd();
#endif
}