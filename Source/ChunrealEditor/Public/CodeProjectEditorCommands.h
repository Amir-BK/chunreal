// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class FCodeProjectEditorCommands : public TCommands<FCodeProjectEditorCommands>
{
public:
	FCodeProjectEditorCommands();

	TSharedPtr<FUICommandInfo> Save;
	TSharedPtr<FUICommandInfo> SaveAll;
	TSharedPtr<FUICommandInfo> InsertTab; //needed to 'steal' the tab key from the editor





	/** Initialize commands */
	virtual void RegisterCommands() override;
};
