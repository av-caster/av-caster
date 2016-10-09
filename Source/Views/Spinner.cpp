/*\
|*|  Copyright 2015-2016 bill-auger <https://github.com/bill-auger/av-caster/issues>
|*|
|*|  This file is part of the AvCaster program.
|*|
|*|  AvCaster is free software: you can redistribute it and/or modify
|*|  it under the terms of the GNU General Public License version 3
|*|  as published by the Free Software Foundation.
|*|
|*|  AvCaster is distributed in the hope that it will be useful,
|*|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|  GNU General Public License for more details.
|*|
|*|  You should have received a copy of the GNU General Public License
|*|  along with AvCaster.  If not, see <http://www.gnu.org/licenses/>.
\*/


//[Headers] You can add your own extra header files here...

#include "../Constants/GuiConstants.h"

//[/Headers]

#include "Spinner.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Spinner::Spinner ()
    : animator(Desktop::getInstance().getAnimator())
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    drawable1 = Drawable::createFromImageData (BinaryData::avcasterlogo128_png, BinaryData::avcasterlogo128_pngSize);

    //[UserPreSize]

  this->mask        = new DrawableImage() ;
  Image    mask_img = Image(Image::ARGB , GUI::SPINNER_MASK_W , GUI::SPINNER_W , true) ;
  Graphics mask_g(mask_img) ;

  mask_g.setColour(Colours::black) ;
  mask_g.fillRect(0                  , 0 , GUI::SPINNER_W , GUI::SPINNER_W) ;
  mask_g.fillRect(GUI::SPINNER_W * 2 , 0 , GUI::SPINNER_W , GUI::SPINNER_W) ;
  this->mask->setImage(mask_img) ;
  addAndMakeVisible(this->mask) ;

    //[/UserPreSize]

    setSize (1, 1);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Spinner::~Spinner()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    drawable1 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Spinner::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0x80000000));
    g.fillRoundedRectangle (18.0f, 18.0f, static_cast<float> (getWidth() - 36), static_cast<float> (getHeight() - 36), 5.000f);

    g.setColour (Colours::black);
    jassert (drawable1 != 0);
    if (drawable1 != 0)
        drawable1->drawWithin (g, Rectangle<float> (proportionOfWidth (0.5000f) - (128 / 2), proportionOfHeight (0.5000f) - (128 / 2), 128, 128),
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Spinner::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void Spinner::broughtToFront()
{
  this->mask->setBounds(GUI::SPINNER_MASK_BEGIN) ;
  this->animator.animateComponent(this->mask            , GUI::SPINNER_MASK_END , 1.0f ,
                                  GUI::SPINNER_MASK_DUR , true                  , 1.0  , 1.0) ;
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Spinner" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers="animator(Desktop::getInstance().getAnimator())"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="1" initialHeight="1">
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="18 18 36M 36M" cornerSize="5" fill="solid: 80000000" hasStroke="0"/>
    <IMAGE pos="50%c 50%c 128 128" resource="BinaryData::avcasterlogo128_png"
           opacity="1" mode="2"/>
  </BACKGROUND>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
