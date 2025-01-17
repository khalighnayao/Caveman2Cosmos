#include "CvGameCoreDLL.h"
#include "CvInfos.h"
#include "CyGlobalContext.h"
#include "CyMap.h"

//
// published python interface for CyGlobalContext
//

void CyGlobalContextPythonInterface3(python::class_<CyGlobalContext>& x)
{
	OutputDebugString("Python Extension Module - CyGlobalContextPythonInterface3\n");

	x
		.def("getMapInfo", &CyGlobalContext::getMapInfo, python::return_value_policy<python::reference_existing_object>())
		.def("switchMap", &CyGlobalContext::switchMap, "void (int)")
		.def("getMapByIndex", &CyGlobalContext::getMapByIndex, python::return_value_policy<python::reference_existing_object>(), "CyMap (int)")

		.def("getAttitudeInfo", &CyGlobalContext::getAttitudeInfo, python::return_value_policy<python::reference_existing_object>(), "AttitudeInfo (int id)")
		.def("getMemoryInfo", &CyGlobalContext::getMemoryInfo, python::return_value_policy<python::reference_existing_object>(), "MemoryInfo (int id)")

		.def("getPlayerOptionsInfo", &CyGlobalContext::getPlayerOptionInfo, python::return_value_policy<python::reference_existing_object>(), "(PlayerOptionsInfoID) - PlayerOptionsInfo for PlayerOptionsInfo")

		.def("getGraphicOptionsInfo", &CyGlobalContext::getGraphicOptionInfo, python::return_value_policy<python::reference_existing_object>(), "(GraphicOptionsInfoID) - GraphicOptionsInfo for GraphicOptionsInfo")

		.def("getNumHurryInfos", &CyGlobalContext::getNumHurryInfos, "() - Total Hurry Infos")

		.def("getNumConceptInfos", &CyGlobalContext::getNumConceptInfos, "int () - NumConceptInfos")
		.def("getConceptInfo", &CyGlobalContext::getConceptInfo, python::return_value_policy<python::reference_existing_object>(), "Concept Info () - Returns info object")

		.def("getNumNewConceptInfos", &CyGlobalContext::getNumNewConceptInfos, "int () - NumNewConceptInfos")
		.def("getNewConceptInfo", &CyGlobalContext::getNewConceptInfo, python::return_value_policy<python::reference_existing_object>(), "New Concept Info () - Returns info object")

		.def("getNumCalendarInfos", &CyGlobalContext::getNumCalendarInfos, "int () - Returns NumCalendarInfos")
		.def("getCalendarInfo", &CyGlobalContext::getCalendarInfo, python::return_value_policy<python::reference_existing_object>(), "CalendarInfo () - Returns Info object")

		.def("getNumGameOptionInfos", &CyGlobalContext::getNumGameOptionInfos, "int () - Returns NumGameOptionInfos")
		.def("getGameOptionInfo", &CyGlobalContext::getGameOptionInfo, python::return_value_policy<python::reference_existing_object>(), "GameOptionInfo () - Returns Info object")

		.def("getNumMPOptionInfos", &CyGlobalContext::getNumMPOptionInfos, "int () - Returns NumMPOptionInfos")
		.def("getMPOptionInfo", &CyGlobalContext::getMPOptionInfo, python::return_value_policy<python::reference_existing_object>(), "MPOptionInfo () - Returns Info object")

		.def("getNumForceControlInfos", &CyGlobalContext::getNumForceControlInfos, "int () - Returns NumForceControlInfos")
		.def("getForceControlInfo", &CyGlobalContext::getForceControlInfo, python::return_value_policy<python::reference_existing_object>(), "ForceControlInfo () - Returns Info object")

		.def("getNumSeasonInfos", &CyGlobalContext::getNumSeasonInfos, "int () - Returns NumSeasonInfos")
		.def("getSeasonInfo", &CyGlobalContext::getSeasonInfo, python::return_value_policy<python::reference_existing_object>(), "SeasonInfo () - Returns Info object")

		.def("getNumDenialInfos", &CyGlobalContext::getNumDenialInfos, "int () - Returns NumDenialInfos")
		.def("getDenialInfo", &CyGlobalContext::getDenialInfo, python::return_value_policy<python::reference_existing_object>(), "DenialInfo () - Returns Info object")
	;
}
