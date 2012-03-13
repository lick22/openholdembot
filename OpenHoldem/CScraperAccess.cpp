#include "stdafx.h"
#include "CScraperAccess.h"

#include "CPreferences.h"
#include "CScraper.h"
#include "CStringMatch.h"
#include "..\CTablemap\CTableMapAccess.h"

#include "MagicNumbers.h"


CScraperAccess *p_scraper_access = NULL;


CScraperAccess::CScraperAccess()
{
}

CScraperAccess::~CScraperAccess()
{
}

// Card Functions
int CScraperAccess::GetPlayerCards(int seat_number, int first_or_second_card)
{
	return p_scraper->card_player(seat_number, first_or_second_card);
}

int CScraperAccess::GetCommonCard(int index_zero_to_four)
{
	return p_scraper->card_common(index_zero_to_four);
}

bool CScraperAccess::IsValidCard(int Card)
{
	if (Card >= 0 && Card < k_number_of_cards_per_deck)
		return true;

	return false;
}

// Button Functions
int CScraperAccess::SearchForButtonNumber(int button_code)
{
	/*
		Searches tablemap labels for button definitions/overrides
		returns the button number if a label is defined
		or the default button number otherwise.
	*/

	int button_number = k_button_undefined;

	// Define a function pointer for the string matching function corresponding to each button_code
	const bool (CStringMatch::*StringMatch)(CString) = NULL;

	switch (button_code)
	{
		// ALLIN
		case k_button_allin:
			StringMatch = &CStringMatch::IsStringAllin;
			break;
		// RAISE
		case k_button_raise:
			StringMatch = &CStringMatch::IsStringRaise;
			break;
		// CALL
		case k_button_call:
			StringMatch = &CStringMatch::IsStringCall;
			break;
		// FOLD
		case k_button_fold:
			StringMatch = &CStringMatch::IsStringFold;
			break;
		// CHECK
		case k_button_check:
			StringMatch = &CStringMatch::IsStringCheck;
			break;
		// SITIN
		case k_button_sitin:
			StringMatch = &CStringMatch::IsStringSitin;
			break;
		// SITOUT
		case k_button_sitout:
			StringMatch = &CStringMatch::IsStringSitout;
			break;
		// LEAVE
		case k_button_leave:
			StringMatch = &CStringMatch::IsStringLeave;
			break;
		default:
			break;
	}

	// If a string verification routine is available
	if (StringMatch)
	{
		// Check if there is a match for any of the corresponding button indexes
		// and save it as the button number
		for (int j = 0; j < k_max_number_of_buttons; j++)
		{
			if ((p_string_match->*StringMatch)(p_scraper->button_label(j)))
			{
				button_number = j;
				break;
			}
		}
	}

	return button_number;
}

CString CScraperAccess::SearchForButtonName(int button_code)
{
	/*
		Searches for a button name
		ATM it only handles the i%dbutton format
	*/

	CString button_name = "";
	int button_number = SearchForButtonNumber(button_code);

	// If the button number is still undefined
	// return the default button number
	if (button_number == k_button_undefined)
		button_number = DefaultButtonNumber(button_code);

	if (button_number != k_button_undefined)
		button_name.Format("i%dbutton", button_number);

	return button_name;
}

bool CScraperAccess::SearchForButtonVisible(int button_code)
{
	/*
		Checks if a button is visible
		i.e. available for taking an action
	*/

	bool button_visible = false;
	int button_number = SearchForButtonNumber(button_code);

	if (button_number != k_button_undefined && p_scraper->GetButtonState(button_number))
		button_visible = true;

	return button_visible;
}

CString CScraperAccess::GetButtonName(int button_number)
{
	/*
		Returns a button name
	*/

	if (button_number == k_button_undefined)
		return "";

	CString button_name = "";
	button_name.Format("i%dbutton", button_number);

	return button_name;
}

bool CScraperAccess::GetButtonVisible(int button_number)
{
	/*
		Checks if a button is visible
		i.e. available for taking an action
	*/

	bool button_visible = false;

	if (button_number != k_button_undefined && p_scraper->GetButtonState(button_number))
		button_visible = true;

	return button_visible;
}

bool CScraperAccess::GetBetpotButtonVisible(int i)
{
	CString betpot_button_state = p_scraper->betpot_button_state(i);

	if (betpot_button_state.MakeLower().Left(4) == "true" ||
		betpot_button_state.MakeLower().Left(2) == "on" ||
		betpot_button_state.MakeLower().Left(3) == "yes" ||
		betpot_button_state.MakeLower().Left(7) == "checked" ||
		betpot_button_state.MakeLower().Left(3) == "lit" )
	{
		return true;
	}

	return false;
}

void CScraperAccess::SetScraperAccessData()
{
	// find button numbers
	_allin_button_number	= SearchForButtonNumber(k_button_allin);
	_raise_button_number	= SearchForButtonNumber(k_button_raise);
	_call_button_number		= SearchForButtonNumber(k_button_call);
	_check_button_number	= SearchForButtonNumber(k_button_check);
	_fold_button_number		= SearchForButtonNumber(k_button_fold);
	_sitin_button_number	= SearchForButtonNumber(k_button_sitin);
	_sitout_button_number	= SearchForButtonNumber(k_button_sitout);
	_leave_button_number	= SearchForButtonNumber(k_button_leave);
	_prefold_button_number	= SearchForButtonNumber(k_button_prefold);
	_autopost_button_number	= SearchForButtonNumber(k_button_autopost);

	// set button names
	button_names[k_autoplayer_function_allin]	= GetButtonName(_allin_button_number);
	button_names[k_autoplayer_function_raise]	= GetButtonName(_raise_button_number);
	button_names[k_autoplayer_function_call]	= GetButtonName(_call_button_number);
	button_names[k_autoplayer_function_check]	= GetButtonName(_check_button_number);
	button_names[k_autoplayer_function_fold]	= GetButtonName(_fold_button_number);
	button_names[k_autoplayer_function_prefold]	= GetButtonName(_prefold_button_number);
	button_names[k_autoplayer_function_sitin]	= GetButtonName(_sitin_button_number);
	button_names[k_autoplayer_function_sitout]	= GetButtonName(_sitout_button_number);
	button_names[k_autoplayer_function_leave]	= GetButtonName(_leave_button_number);
	for (int i = k_betpot_min; i <= k_betpot_max; i++)
		button_names[i] = k_betpot_button_name[k_betpot_index(i)];

	// hardcoded
	_i3_button_name = "i3button";
	_i3_slider_name = "i3slider";
	_i3_handle_name = "i3handle";
	_i86_button_name = "i86button";

	CString button_name = "";
	for (int i = 0; i < k_max_number_of_i86X_buttons; i++)
	{
		button_name.Format("i%dbutton", k_button_i86*k_max_number_of_i86X_buttons + i);
		_i86X_button_name[i] = button_name;
	}

	visible_buttons[k_autoplayer_function_allin]	= GetButtonVisible(_allin_button_number);
	visible_buttons[k_autoplayer_function_raise]	= GetButtonVisible(_raise_button_number);
	visible_buttons[k_autoplayer_function_call]		= GetButtonVisible(_call_button_number);
	visible_buttons[k_autoplayer_function_check]	= GetButtonVisible(_check_button_number);
	visible_buttons[k_autoplayer_function_fold]		= GetButtonVisible(_fold_button_number);
	visible_buttons[k_autoplayer_function_prefold]	= GetButtonVisible(_prefold_button_number);
	visible_buttons[k_autoplayer_function_sitin]	= GetButtonVisible(_sitin_button_number);
	visible_buttons[k_autoplayer_function_sitout]	= GetButtonVisible(_sitout_button_number);
	visible_buttons[k_autoplayer_function_leave]	= GetButtonVisible(_leave_button_number);
	for (int i = k_betpot_min; i <= k_betpot_max; i++)
		visible_buttons[i] = GetBetpotButtonVisible(k_betpot_index(i));

	// hardcoded 
	i3_button_visible = GetButtonVisible(k_button_i3);
	i86_button_visible = GetButtonVisible(k_button_i86);

	for (int i = 0; i < k_max_number_of_i86X_buttons; i++)
		i86X_button_visible[i] = GetButtonVisible(k_button_i86*k_max_number_of_i86X_buttons + i);
}

void CScraperAccess::GetNeccessaryTablemapObjects()
{
	SetScraperAccessData();

	// Defined
	for (int i = k_autoplayer_function_allin; i < k_number_of_autoplayer_functions; i++)
	{
		defined_buttons[i] = p_tablemap_access->GetButtonRect(button_names[i], &action_buttons[i]);
	}

	i3_button_defined		= p_tablemap_access->GetButtonRect("i3button", &i3_button);
	i3_edit_defined			= p_tablemap_access->GetTableMapRect("i3edit", &i3_edit_region);
	i3_slider_defined		= p_tablemap_access->GetTableMapRect("i3slider", &i3_slider_region);
	i3_handle_defined		= p_tablemap_access->GetTableMapRect("i3button", &i3_handle_region);
	i86_button_defined		= p_tablemap_access->GetButtonRect("i86button", &i86_button);

	CString i86X_button_name = "";
	for (int i = 0; i < k_max_number_of_i86X_buttons; i++)
	{
		i86X_button_name.Format("i86%dbutton", i);
		i86X_button_defined[i]     = p_tablemap_access->GetButtonRect(i86X_button_name, &i86X_button[i]);
	}
	
	// Available
	for (int i = k_autoplayer_function_allin; i < k_number_of_autoplayer_functions; i++)
	{
		available_buttons[i] = defined_buttons[i] && visible_buttons[i];
	}

	i3_button_available		= i3_button_defined && i3_button_visible;
	i86_button_available	= i86_button_defined && i86_button_visible;

	for (int i = 0; i < k_max_number_of_i86X_buttons; i++)
	{
		i86X_button_available[i]   = i86X_button_defined[i] && i86X_button_visible[i];
	}

	allin_option_available = false;

	if (i3_button_available)
		allin_option_available = true;
	if (i3_button_visible && available_buttons[k_autoplayer_function_allin])
		allin_option_available = true;
	if (i3_button_visible && i3_edit_defined)
		allin_option_available = true;
	if (i3_button_visible && i3_slider_defined && i3_handle_defined)
		allin_option_available = true;
}

int CScraperAccess::NumberOfVisibleButtons()
{
	// Buttons for playing actions, e.g. fold or allin.
	// There have to be at least 2 to make it our turn.
	int number_of_visible_buttons = 0
		+ (available_buttons[k_autoplayer_function_allin] ? 1 : 0)
		+ (available_buttons[k_autoplayer_function_raise] ? 1 : 0)
		+ (available_buttons[k_autoplayer_function_call]  ? 1 : 0)
		+ (available_buttons[k_autoplayer_function_check] ? 1 : 0)
		+ (available_buttons[k_autoplayer_function_fold]  ? 1 : 0);

	return number_of_visible_buttons;
}