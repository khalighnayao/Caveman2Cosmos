#include "WorkerAI.h"

#include "CvGameAI.h"
#include "CvPlayerAI.h"
#include "CvSelectionGroup.h"
#include "CvUnitAI.h"

void WorkerAI::WorkerMove(CvUnit* worker)
{
	PROFILE_FUNC();

	if (AI_selectStatus(true))
	{
		return;
	}
	// If worker (or captive) cannot defend itself, and is outside own borders.
	if (!Worker_CanDefend() && IsAbroad())
	{
		// Look for a local group we can join to be safe!
		AI_setLeaderPriority(LEADER_PRIORITY_MAX); // We do want to take control (otherwise other unit decides where this worker goes, and can go further away)
		if (AI_group(GroupingParams().withUnitAI(UNITAI_ATTACK).ignoreFaster().ignoreOwnUnitType().maxPathTurns(1)))
		{
			return;
		}
		if (AI_group(GroupingParams().withUnitAI(UNITAI_HUNTER).ignoreOwnUnitType().maxPathTurns(1)))
		{
			return;
		}
		if (AI_group(GroupingParams().withUnitAI(UNITAI_ATTACK_CITY).ignoreFaster().ignoreOwnUnitType().maxPathTurns(1)))
		{
			return;
		}
		if (AI_group(GroupingParams().withUnitAI(UNITAI_COUNTER).ignoreFaster().ignoreOwnUnitType().maxPathTurns(1)))
		{
			return;
		}

		AI_setLeaderPriority(-1); // We didn't get to group so back to normal

		// Nobody can join us and we cannot join anyone else - run for it!
		if (AI_retreatToCity())
		{
			return;
		}
	}
	//ls612: Combat Worker Danger Evaluation
	const bool bWorkerDanger =
		(IsAbroad() && GET_PLAYER(getOwner()).AI_isPlotThreatened(plot(), 2) ||
			!IsAbroad() && exposedToDanger(plot(), 80, false));
	if (canDefend() && GetNumberOfUnitsInGroup() == 1 && bWorkerDanger) {
		// in this order, either retreat to safety, or go into a city
		if (AI_safety() || AI_retreatToCity()) {
			return;
		}
	}

	if (!isHuman() && !isNPC())
	{
		if (AI_workerReleaseDefenderIfNotNeeded())
		{
			return;
		}
	}

	if (AI_construct(MAX_INT, MAX_INT, 0, true))
	{
		OutputDebugString(CvString::format("%S (%d) chooses to head off to construct\n", getDescription().c_str(), m_iID).c_str());
		return;
	}
	if (AI_upgradeWorker())
	{
		return;
	}

	if (AI_hurry(true))
	{
		return;
	}

	if (AI_outcomeMission())
	{
		OutputDebugString(CvString::format("%S (%d) chooses to head off to do an outcome mission\n", getDescription().c_str(), m_iID).c_str());
		return;
	}

	if (!Worker_CanDefend() && (isHuman() && GET_PLAYER(getOwner()).AI_isPlotThreatened(plot(), 2) || !isHuman() && AI_workerNeedsDefender(plot())) && AI_retreatToCity() /*XXX maybe not do this??? could be working productively somewhere else...*/)
	{
		return;
	}

	bool bCanRoute = canBuildRoute();
	// Afforess 02/17/10
	// Workboats don't build Sea Tunnels over Resources
	if (bCanRoute && getDomainType() != DOMAIN_SEA && !IsAbroad())
	{
		const BonusTypes eNonObsoleteBonus = plot()->getNonObsoleteBonusType(getTeam());

		if (NO_BONUS != eNonObsoleteBonus && !plot()->isConnectedToCapital() && NO_IMPROVEMENT != plot()->getImprovementType() && GC.getImprovementInfo(plot()->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus) && AI_connectPlot(plot()))
		{
			return;
		}
	}

	CvPlot* pBestBonusPlot = NULL;
	BuildTypes eBestBonusBuild = NO_BUILD;
	int iBestBonusValue = 0;

	if (AI_improveBonus(25, &pBestBonusPlot, &eBestBonusBuild, &iBestBonusValue))
	{
		return;
	}

	if (bCanRoute && !isNPC() && AI_connectCity())
	{
		return;
	}

	// Afforess - worker financial trouble check
	if (!isHuman() && AI_getUnitAIType() == UNITAI_WORKER && GET_PLAYER(getOwner()).AI_isFinancialTrouble()) // not evaluated 
	{
		const int iWorkers = GET_PLAYER(getOwner()).AI_totalUnitAIs(UNITAI_WORKER);

		if (iWorkers > 3 && iWorkers > 2 * GET_PLAYER(getOwner()).getNumCities() && GET_PLAYER(getOwner()).getUnitUpkeepCivilianNet() > 0)
		{
			if (gUnitLogLevel > 2)
			{
				logBBAI(
					"%S's %S at (%d,%d) is disbanding itself due to large number of workers available, and financial trouble.",
					GET_PLAYER(getOwner()).getCivilizationDescription(0), getName(0).GetCString(), getX(), getY());
			}
			if (getUpkeep100() > 100)
			{
				scrap();
			}

			return;
		}
	}

	CvCity* pCity = NULL;

	if (!IsAbroad())
	{
		pCity = plot()->getPlotCity(); //get city on plot
		if (pCity == NULL) //if city is not on same plot
		{
			pCity = plot()->getWorkingCity(); //get city that is owning this plot
		}
	}

	if (pCity != NULL && pCity->AI_getWorkersNeeded() > 0 && (plot()->isCity() || pCity->AI_getWorkersNeeded() < (1 + pCity->AI_getWorkersHave() * 2) / 3) && AI_improveCity(pCity))
	{
		return;
	}

	if (AI_improveLocalPlot(2, pCity))
	{
		return;
	}
	// Super Forts end

	/*
	if (bCanRoute && isBarbarian() && AI_connectCity())
	{
		return;
	}
	*/

	bool bNextCity = false;

	if (pCity == NULL || pCity->AI_getWorkersNeeded() == 0 || pCity->AI_getWorkersHave() > pCity->AI_getWorkersNeeded() + 1)
	{
		if (pBestBonusPlot != NULL && iBestBonusValue >= 15 && AI_improvePlot(pBestBonusPlot, eBestBonusBuild))
		{
			return;
		}

		if (AI_nextCityToImprove(pCity))
		{
			return;
		}
		bNextCity = true;
	}

	if (pBestBonusPlot != NULL && AI_improvePlot(pBestBonusPlot, eBestBonusBuild))
	{
		return;
	}

	if (pCity != NULL && AI_improveCity(pCity))
	{
		return;
	}

	if (!bNextCity && AI_nextCityToImprove(pCity))
	{
		return;
	}

	if (bCanRoute)
	{
		if (AI_routeTerritory(true))
		{
			return;
		}

		if (AI_connectBonus(false))
		{
			return;
		}

		if (AI_routeCity())
		{
			return;
		}
	}

	if (AI_improveLocalPlot(3, NULL))
	{
		return;
	}

	if (AI_irrigateTerritory())
	{
		return;
	}
	bool bBuildFort = false;

	// Super Forts begin *canal* *choke*
	if (0 == GC.getGame().getSorenRandNum(5, "AI Worker build Fort with Priority"))
	{
		const CvPlayerAI& player = GET_PLAYER(getOwner());
		const bool bCanal = player.countNumCoastalCities() > 0;
		const bool bAirbase = player.AI_totalUnitAIs(UNITAI_PARADROP) || player.AI_totalUnitAIs(UNITAI_ATTACK_AIR) || player.AI_totalUnitAIs(UNITAI_MISSILE_AIR);

		if (AI_fortTerritory(bCanal, bAirbase))
		{
			return;
		}
		bBuildFort = bCanal && bAirbase;
	}

	// Super Forts begin *canal* *choke*
	if (!bBuildFort && AI_fortTerritory(true, true /*bCanal, bAirbase*/))
	{
		return;
	}
	// Super Forts end

	//if (AI_StrategicForts())
	//{
	//	return;
	//}

	if (bCanRoute && AI_routeTerritory())
	{
		return;
	}

	if (!isHuman() || isAutomated() && !GET_TEAM(getTeam()).isAtWar())
	{
		if (!isHuman() || getGameTurnCreated() < GC.getGame().getGameTurn())
		{
			if (AI_nextCityToImproveAirlift())
			{
				return;
			}
		}
		if (!isHuman())
		{
			// Fill up boats which already have workers
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_WORKER, -1, -1, -1, -1, MOVE_SAFE_TERRITORY))
			{
				return;
			}

			// Avoid filling a galley which has just a settler in it, reduce chances for other ships
			if (GC.getGame().isOption(GAMEOPTION_SIZE_MATTERS))
			{
				if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, (2 * GET_PLAYER(getOwner()).getBestUnitTypeCargoVolume(UNITAI_WORKER)), -1, -1, MOVE_SAFE_TERRITORY))
				{
					return;
				}
			}
			else if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, 2, -1, -1, MOVE_SAFE_TERRITORY))
			{
				return;
			}
		}
	}


	if (!isHuman() && !isNPC())
	{
		if (!IsAbroad() && AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 2, -1, -1, 0, MOVE_SAFE_TERRITORY))
		{
			return;
		}
	}

	if (!isHuman() && AI_getUnitAIType() == UNITAI_WORKER)
	{
		const int iWorkers = GET_PLAYER(getOwner()).AI_totalUnitAIs(UNITAI_WORKER);

		if (iWorkers > 3 && iWorkers > 5 * GET_PLAYER(getOwner()).getNumCities() && GET_PLAYER(getOwner()).getUnitUpkeepCivilianNet() > 0)
		{
			if (gUnitLogLevel > 2)
			{
				logBBAI("	%S's %S at (%d,%d) is disbanding itself due to large number of workers available", GET_PLAYER(getOwner()).getCivilizationDescription(0), getName(0).GetCString(), getX(), getY());
			}
			scrap();
			return;
		}
	}

	if (AI_retreatToCity(false, true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (getGroup()->isStranded() && AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, -1, -1, -1, MOVE_NO_ENEMY_TERRITORY, 1))
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
}

void CvUnitAI::AI_barbAttackMove()
{
	PROFILE_FUNC();

	if (AI_selectStatus(true))
	{
		return;
	}

	if (AI_anyAttack(0, 60))
	{
		return;
	}

	if (AI_anyAttack(1, 80))
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->isGoody())
	{
		if (AI_anyAttack(1, 90))
		{
			return;
		}

		if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ATTACK, -1, NULL, getOwner()) == 1 && getGroup()->getNumUnits() == 1)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (GC.getGame().getSorenRandNum(2, "AI Barb") == 0 && AI_pillageRange(1))
	{
		return;
	}

	if (AI_anyAttack(1, 20))
	{
		return;
	}

	if (GC.getGame().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		if (AI_pillageRange(4))
		{
			return;
		}

		if (AI_cityAttack(3, 10))
		{
			return;
		}

		if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
		{
			if (AI_groupMergeRange(UNITAI_ATTACK, 1, true, true, true))
			{
				return;
			}

			if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 3, true, true, true))
			{
				return;
			}

			if (AI_goToTargetCity(0, MAX_BARB_TARGET_CITY_RANGE))
			{
				return;
			}
		}
	}
	else if (GC.getGame().getNumCivCities() > (GC.getGame().countCivPlayersAlive() * 3))
	{
		if (AI_cityAttack(1, 15))
		{
			return;
		}

		if (AI_pillageRange(3))
		{
			return;
		}

		if (AI_cityAttack(2, 10))
		{
			return;
		}

		if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
		{
			if (AI_groupMergeRange(UNITAI_ATTACK, 1, true, true, true))
			{
				return;
			}

			if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 3, true, true, true))
			{
				return;
			}

			if (AI_goToTargetCity(0, MAX_BARB_TARGET_CITY_RANGE))
			{
				return;
			}
		}
	}
	else if (GC.getGame().getNumCivCities() > (GC.getGame().countCivPlayersAlive() * 2))
	{
		if (AI_pillageRange(2))
		{
			return;
		}

		if (AI_cityAttack(1, 10))
		{
			return;
		}
	}

	if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 1))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_guardCity(false, true, 2))
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}
