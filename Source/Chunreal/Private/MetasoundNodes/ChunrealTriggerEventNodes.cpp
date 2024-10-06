
#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"

#include "MetasoundStandardNodesNames.h"
#include "MetasoundTrigger.h"

#include "Chunreal.h"
#include "ChuckInstance.h"

//nodes that can translate triggers to chuck events and the other way around