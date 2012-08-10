/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef BUMPTOP_TOOLTIPMANAGER_H_
#define BUMPTOP_TOOLTIPMANAGER_H_

#include "BumpTop/Singleton.h"

class BumpPile;
class ToolTipOverlay;
class ToolTipManager {
  SINGLETON_HEADER(ToolTipManager)
 public:
  explicit ToolTipManager();

  void showTaskbarTooltip();
  void hideTaskbarTooltip();
  void showGriddedPileTooltip(BumpPile* pile);
  void hideGriddedPileTooltip();
  void showPileFlipTooltip(BumpPile* pile);
  void hidePileFlipTooltip();
  void showNamePileTooltip(BumpPile* pile);
  void hideNamePileTooltip();

  void deleteTooltip(ToolTipOverlay* tooltip_);

 protected:
  ToolTipOverlay* taskbar_tooltip_;
  ToolTipOverlay* gridded_pile_tooltip_;
  ToolTipOverlay* pile_flip_tooltip_;
  ToolTipOverlay* name_pile_tooltip_;

  bool taskbar_tooltip_will_be_deleted_;
  bool gridded_pile_tooltip_will_be_deleted_;
  bool pile_flip_tooltip_will_be_deleted_;
  bool name_pile_tooltip_will_be_deleted_;
};

#endif  // BUMPTOP_TOOLTIPMANAGER_H_
