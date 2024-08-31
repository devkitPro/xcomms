#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Box.H>

#include <xboo.h>
#include <config.h>
#include <parport.h>

Fl_Double_Window* OptionsDlg;
Fl_Choice *Port, *Hardware;
Fl_Round_Button	*Verify;
Fl_Round_Button	*DoubleSpeed;
Fl_Round_Button	*HeaderFix;
Fl_Counter *TxDelay;

//-----------------------------------------------------------------------------
void OptionsOK()
//-----------------------------------------------------------------------------
{
	SetPortAddress(Port->value()?0x378:0x278);
	SetVerify(Verify->value());
//	SetBurst(DoubleSpeed->value());
	SetDelay((int)TxDelay->value());

	XcommsCfg.MBV2 = Hardware->value();
	XcommsCfg.headerfix = HeaderFix->value();
	delete OptionsDlg;
}

//-----------------------------------------------------------------------------
void OptionsCancel()
//-----------------------------------------------------------------------------
{
	delete OptionsDlg;
}

//-----------------------------------------------------------------------------
void Options()
//-----------------------------------------------------------------------------
{

	{ Fl_Double_Window*	o =	new	Fl_Double_Window(225, 250, "Options");
		OptionsDlg = o;
		o->align(FL_ALIGN_TOP_LEFT);
	{ Fl_Choice* o = Port = new Fl_Choice(125, 13, 90, 20);
		o->down_box(FL_BORDER_BOX);
		o->labeltype(FL_NO_LABEL);
		o->labelsize(12);
		o->textsize(12);
		o->add("0x278|0x378");
		int index = 0;
		if (GetPortAddress() == 0x378) index = 1;
		o->value(index);
	}
	{ Fl_Choice* o = Hardware = new Fl_Choice(125, 40, 90, 20);
		o->down_box(FL_BORDER_BOX);
		o->labeltype(FL_NO_LABEL);
		o->labelsize(12);
		o->textsize(12);
		o->add("Xboo|MBV2");
		o->value(XcommsCfg.MBV2);
	}


	{ Fl_Round_Button* o = Verify =	new	Fl_Round_Button(195, 75, 20, 25, "button");
		o->down_box(FL_ROUND_DOWN_BOX);
		o->labeltype(FL_NO_LABEL);
		o->value(GetVerify());
	}
/*	{ Fl_Round_Button* o = DoubleSpeed = new Fl_Round_Button(195, 75, 20, 25, "button");
		o->down_box(FL_ROUND_DOWN_BOX);
		o->labeltype(FL_NO_LABEL);
		o->value(GetBurst());
	}
*/	{ Fl_Round_Button* o = HeaderFix = new Fl_Round_Button(195, 105, 20, 25, "button");
		o->down_box(FL_ROUND_DOWN_BOX);
		o->labeltype(FL_NO_LABEL);
		o->value(XcommsCfg.headerfix);
	}
	{ Fl_Button *o = new	Fl_Button(25, 200, 60, 25, "Cancel");
  		o->callback((Fl_Callback*)OptionsCancel);
	}
	{ Fl_Button *o = new	Fl_Button(140, 200,	60,	25,	"OK");
   		o->callback((Fl_Callback*)OptionsOK);

	}
	{ Fl_Counter* o	= TxDelay =	new	Fl_Counter(150,	150, 60, 25, "counter:");
		o->labeltype(FL_NO_LABEL);
		o->minimum(0);
		o->maximum(10);
		o->step(1);
		o->value(GetDelay());
		o->align(FL_ALIGN_BOTTOM_RIGHT);
		o->type(FL_SIMPLE_COUNTER);
	}
	{ Fl_Box* o	= new Fl_Box(10, 13, 85, 22, "Port Address");
		o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	}
	{ Fl_Box* o	= new Fl_Box(10, 40, 85, 22, "Hardware");
		o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	}
	{ Fl_Box* o	= new Fl_Box(10, 75, 140, 25, "Verify Main Data");
		o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	}
/*	{ Fl_Box* o	= new Fl_Box(10, 75, 140, 25, "Double Speed Burst");
		o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	}
*/	{ Fl_Box* o	= new Fl_Box(10, 105, 140, 25, "Fix GBA Header");
		o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	}
	{ Fl_Box* o	= new Fl_Box(10, 150, 130, 25, "Transfer Delay");
		o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	}
		o->end();
	}

	OptionsDlg->set_modal();
	OptionsDlg->show();
	while( OptionsDlg->shown() ) Fl::wait();

}
