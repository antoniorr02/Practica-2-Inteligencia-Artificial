#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>

/**
 * Devuelve si una ubicación en el mapa es transitable para el agente.
*/
bool CasillaTransitable (const ubicacion &x, const vector<vector<unsigned char>> &mapa) {
	return (mapa[x.f][x.c] != 'P' && mapa[x.f][x.c] != 'M');
}

/**
 * Devuelve la ubicación siguiente según el avance del agente.
*/
ubicacion NextCasilla (const ubicacion &pos) {
	ubicacion next = pos;
	switch (pos.brujula) {
		case norte:
			next.f = pos.f - 1;
		break;
		case noreste:
			next.f = pos.f - 1;
			next.c = pos.c + 1;
		break;
		case este:
			next.c = pos.c + 1;
		break;
		case sureste:
			next.f = pos.f + 1;
			next.c = pos.c + 1;
		break;
		case sur:
			next.f = pos.f + 1;
		break;
		case suroeste:
			next.f = pos.f + 1;
			next.c = pos.c - 1;
		break;
		case oeste:
			next.c = pos.c - 1;
		break;
		case noroeste:
			next.f = pos.f - 1;
			next.c = pos.c - 1;
		break;
	}
	return next;
}

/**
 * Pone  a cero todos los elementos de una matriz
*/
void AnularMatriz(vector<vector<unsigned char>> &matriz) {
	for (int i = 0; i < matriz.size(); i++) {
		for (int j = 0; j < matriz.size(); j++) {
			matriz[i][j] = 0;
		}
	}
}

/////////////////////////////////// NIVEL 0 ////////////////////////////////////////

/**
 * Devuelve el estado que se genera si el agente puede avanzar.
 * Si no puede avanzar, se devuelve como salida el mismo estado de entrada.
*/
stateNO apply (const Action &a, const stateNO &st, const vector<vector<unsigned char>> &mapa) {
	stateNO st_result = st;
	ubicacion sig_ubicacion;
	switch (a) {
		case actFORWARD:
			sig_ubicacion = NextCasilla(st.jugador);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.sonambulo.f && sig_ubicacion.c == st.sonambulo.c)) {
				st_result.jugador = sig_ubicacion;
			}
		break;
		case actTURN_L:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+6)%8);
		break;
		case actTURN_R:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+2)%8);
		break;
	}
	return st_result;
}

void ComportamientoJugador::VisualizaPlan0(const stateNO &st, const list<Action> &plan) {
	AnularMatriz(mapaConPlan);
	stateNO cst = st;

	auto it = plan.begin();
	while (it != plan.end()) {
		switch (*it){
			case actFORWARD:
				cst.jugador = NextCasilla(cst.jugador);
				mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			break;
			case actTURN_L:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+6)%8);
			break;
			case actTURN_R:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+2)%8);
			break;
			case actSON_FORWARD:
				cst.sonambulo = NextCasilla(cst.sonambulo);
				mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
			case actSON_TURN_SL:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+7)%8);
			break;
			case actSON_TURN_SR:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+1)%8);
			break;
		}
		it++;
	}
}

/**
 * Búsqueda en anchura para el jugador (nivel 0).
*/
list<Action> AnchuraSoloJugador(const stateNO &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeNO current_node;
	current_node.st = inicio;
	list<nodeNO> frontier;
	set<nodeNO> explored;
	list<Action> plan;
	bool SolutionFound = (current_node.st.jugador.f == final.f and current_node.st.jugador.c == final.c);
	frontier.push_back(current_node);

	while (!frontier.empty() and !SolutionFound) {
		frontier.pop_front();
		explored.insert(current_node);

		// Generar hijo actFORWARD
		nodeNO child_forward = current_node;
		child_forward.st = apply(actFORWARD, current_node.st, mapa);
		if (child_forward.st.jugador.f == final.f and child_forward.st.jugador.c == final.c) {
			child_forward.secuencia.push_back(actFORWARD);
			current_node = child_forward;
			SolutionFound = true;
		} else if (explored.find(child_forward) == explored.end()) {
			child_forward.secuencia.push_back(actFORWARD);
			frontier.push_back(child_forward);
		}

		if (!SolutionFound) {
			// Generar hijo actTURN_L
			nodeNO child_turnl = current_node;
			child_turnl.st = apply(actTURN_L, current_node.st, mapa);
			if (explored.find(child_turnl) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push_back(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeNO child_turnr = current_node;
			child_turnr.st = apply(actTURN_R, current_node.st, mapa);
			if (explored.find(child_turnr) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push_back(child_turnr);
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.front();
			while (!frontier.empty() && explored.find(current_node) != explored.end()) {
				frontier.pop_front();
				current_node = frontier.front();
			}

		}
	}

	if(SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}

/////////////////////////////////// NIVEL 1 ////////////////////////////////////////

/**
 * Devuelve el estado que se genera si el agente puede avanzar.
 * Si no puede avanzar, se devuelve como salida el mismo estado de entrada.
*/
stateN1 apply_1 (const Action &a, const stateN1 &st, const vector<vector<unsigned char>> &mapa) {
	stateN1 st_result = st;
	ubicacion sig_ubicacion;
	switch (a) {
		case actFORWARD:
			sig_ubicacion = NextCasilla(st.jugador);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.sonambulo.f && sig_ubicacion.c == st.sonambulo.c)) {
				st_result.jugador = sig_ubicacion;
			}
		break;
		case actTURN_L:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+6)%8);
		break;
		case actTURN_R:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+2)%8);
		break;
		case actSON_FORWARD:
			sig_ubicacion = NextCasilla(st.sonambulo);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.jugador.f && sig_ubicacion.c == st.jugador.c)) {
				st_result.sonambulo = sig_ubicacion;
			}
		break;
		case actSON_TURN_SL:
			st_result.sonambulo.brujula = static_cast<Orientacion>((st_result.sonambulo.brujula+7)%8);
		break;
		case actSON_TURN_SR:
			st_result.sonambulo.brujula = static_cast<Orientacion>((st_result.sonambulo.brujula+1)%8);
		break;
	}
	return st_result;
}

void ComportamientoJugador::VisualizaPlan1 (const stateN1 &st, const list<Action> &plan) {
	AnularMatriz(mapaConPlan);
	stateN1 cst = st;

	auto it = plan.begin();
	while (it != plan.end()) {
		switch (*it){
			case actFORWARD:
				cst.jugador = NextCasilla(cst.jugador);
				mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			break;
			case actTURN_L:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+6)%8);
			break;
			case actTURN_R:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+2)%8);
			break;
			case actSON_FORWARD:
				cst.sonambulo = NextCasilla(cst.sonambulo);
				mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
			case actSON_TURN_SL:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+7)%8);
			break;
			case actSON_TURN_SR:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+1)%8);
			break;
		}
		it++;
	}
}

/**
 * Comprueba si el sonámbulo está en el campo de visión del jugador.
*/
bool SonambuloAlaVista(const stateN1 &st) {
	bool encontrado = false;
	bool pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8, pos9, pos10, pos11, pos12, pos13, pos14, pos15; 
	switch (st.jugador.brujula) {
		case norte:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos7 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos11 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos13 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos14 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case noreste:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos5 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos8 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos10 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos11 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos14 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos15 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case este:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos6 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos11 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos12 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos14 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case sureste:
			pos1 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos4 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos9 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos11 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos14 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
		break;
		case sur:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos7 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos11 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos13 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos14 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case suroeste:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos5 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos8 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos10 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos11 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos14 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos15 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case oeste:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos6 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos11 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos12 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos14 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case noroeste:
			pos1 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos4 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos9 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos11 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos14 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
		break;
		default:
			return false; // Hay un test que pone orientación 8, entonces se inicializan variables pos mal.
			// Otra opcion es inicializar todas las variables pos, que quizá sería más correcto. En cualquier caso esta solución es valida.
		break;
	}
	if (pos1 || pos2 || pos3 || pos4 || pos5 || pos6 || pos7 || pos8 || pos9 || pos10 || pos11 || pos12 || pos13 || pos14 || pos15) {
		encontrado = true;
	}

	return encontrado;
}

/**
 * Anchura para el jugador y el sonámbulo. (Nivel 1)
 * NOTA: La función SonambuloAlaVista comprueba que el sonámbulo esté en el campo de visión
 * del jugador, para ello hago un switch y según la orientación del jugador miro si alguna de
 * las posiciones del campo de visión coincide con la del sonámbulo.
*/
list<Action> AnchuraAmbos(const Sensores sensores, const stateN1 &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeN1 current_node;
	current_node.st = inicio;
	list<nodeN1> frontier;
	set<nodeN1> explored;
	list<Action> plan;
	bool SolutionFound =  (current_node.st.sonambulo.f == final.f and current_node.st.sonambulo.c == final.c);
	bool encontrado = false;

	frontier.push_back(current_node);

	while (!frontier.empty() && !SolutionFound) {
		frontier.pop_front();
		explored.insert(current_node);
		encontrado = SonambuloAlaVista(current_node.st);

		if (encontrado) {
			// Generar hijo actSON_FORWARD
			nodeN1 child_SON_forward = current_node;
			child_SON_forward.st = apply_1(actSON_FORWARD, current_node.st, mapa);
			if (child_SON_forward.st.sonambulo.f == final.f and child_SON_forward.st.sonambulo.c == final.c) {
				child_SON_forward.secuencia.push_back(actSON_FORWARD);
				current_node = child_SON_forward;
				SolutionFound = true;
			}else if (explored.find(child_SON_forward) == explored.end()) {
				child_SON_forward.secuencia.push_back(actSON_FORWARD);
				frontier.push_back(child_SON_forward);

			}
		}

		if (!SolutionFound) {

			// Generar hijo actFORWARD
			nodeN1 child_forward = current_node;
			child_forward.st = apply_1(actFORWARD, current_node.st, mapa);
			if (explored.find(child_forward) == explored.end()) {
				child_forward.secuencia.push_back(actFORWARD);
				frontier.push_back(child_forward);
			}
			// Generar hijo actTURN_L
			nodeN1 child_turnl = current_node;
			child_turnl.st = apply_1(actTURN_L, current_node.st, mapa);
			if (explored.find(child_turnl) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push_back(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeN1 child_turnr = current_node;
			child_turnr.st = apply_1(actTURN_R, current_node.st, mapa);
			if (explored.find(child_turnr) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push_back(child_turnr);
			}

			if (encontrado) {
				// Generar hijo actTURN_SL
				nodeN1 child_turnsl = current_node;
				child_turnsl.st = apply_1(actSON_TURN_SL, current_node.st, mapa);
				if (explored.find(child_turnsl) == explored.end()) {
					child_turnsl.secuencia.push_back(actSON_TURN_SL);
					frontier.push_back(child_turnsl);
				}
				// Generar hijo actTURN_SR
				nodeN1 child_turnsr = current_node;
				child_turnsr.st = apply_1(actSON_TURN_SR, current_node.st, mapa);
				if (explored.find(child_turnsr) == explored.end()) {
					child_turnsr.secuencia.push_back(actSON_TURN_SR);
					frontier.push_back(child_turnsr);
				}
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.front();
			while (!frontier.empty() && explored.find(current_node) != explored.end()) {
				frontier.pop_front();
				if(!frontier.empty())
					current_node = frontier.front();
			}
		}
	}

	if (SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}

/////////////////////////////////// NIVEL 2 ////////////////////////////////////////

int CalcularCoste(const Action &a, stateN2 &st, char tipocasilla) {
	int coste=1;
	switch (a) 	{
		case actFORWARD:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_jugador) {
						coste = 100;
					} else {
						coste = 10;
					}
				break;
				case 'B':
					if (!st.zapatillas_jugador) {
						coste = 50;
					} else {
						coste = 15;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
				}
		break;
		case actTURN_L:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_jugador) {
						coste = 25;
					} else {
						coste = 5;
					}
				break;
				case 'B':
					if (!st.zapatillas_jugador) {
						coste = 5;
					} else {
						coste = 1;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
			}
		break;
		case actTURN_R:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_jugador) {
						coste = 25;
					} else {
						coste = 5;
					}
				break;
				case 'B':
					if (!st.zapatillas_jugador) {
						coste = 5;
					} else {
						coste = 1;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
			}
	}

	return coste;
}

/**
 * Devuelve el estado que se genera si el agente puede avanzar.
 * Si no puede avanzar, se devuelve como salida el mismo estado de entrada.
*/
stateN2 apply_2(const Action &a, stateN2 &st, const vector<vector<unsigned char>> &mapa) {
	char tipo_casilla_actual = mapa[st.jugador.f][st.jugador.c];
	if (tipo_casilla_actual == 'K') {
		st.bikini_jugador = true;
		st.zapatillas_jugador = false;
	}
	if (tipo_casilla_actual == 'D') {
		st.zapatillas_jugador = true;
		st.bikini_jugador = false;
	}
	stateN2 st_result = st; // Ya tenemos el coste acumulado en el nuevo estado.
	ubicacion sig_ubicacion;

	switch (a) {
		case actFORWARD:
			sig_ubicacion = NextCasilla(st.jugador);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.sonambulo.f && sig_ubicacion.c == st.sonambulo.c)) {
				st_result.jugador = sig_ubicacion;
			}
		break;
		case actTURN_L:
			st_result.jugador.brujula = static_cast<Orientacion>((st.jugador.brujula+6)%8);
		break;
		case actTURN_R:
			st_result.jugador.brujula = static_cast<Orientacion>((st.jugador.brujula+2)%8);
		break;
	}
	return st_result;
}

void ComportamientoJugador::VisualizaPlan2 (const stateN2 &st, const list<Action> &plan) {
	AnularMatriz(mapaConPlan);
	stateN2 cst = st;

	auto it = plan.begin();
	while (it != plan.end()) {
		switch (*it){
			case actFORWARD:
				cst.jugador = NextCasilla(cst.jugador);
				mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			break;
			case actTURN_L:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+6)%8);
			break;
			case actTURN_R:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+2)%8);
			break;
			case actSON_FORWARD:
				cst.sonambulo = NextCasilla(cst.sonambulo);
				mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
			case actSON_TURN_SL:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+7)%8);
			break;
			case actSON_TURN_SR:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+1)%8);
			break;
		}
		it++;
	}
}

/**
 * Solución óptima para el jugador (nivel 2).
*/
list<Action> DjikstraJugador(const stateN2 &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeN2 current_node;
	current_node.st = inicio;
	priority_queue<nodeN2> frontier;
	set<stateN2> explored;
	list<Action> plan;
	bool SolutionFound = (current_node.st.jugador.f == final.f and current_node.st.jugador.c == final.c);
	frontier.push(current_node);

	while (!frontier.empty() and !SolutionFound) {
		frontier.pop();
		explored.insert(current_node.st);

		if (current_node.st.jugador.f == final.f and current_node.st.jugador.c == final.c)
			SolutionFound = true;

		if (!SolutionFound) {

			// Generar hijo actFORWARD
			nodeN2 child_forward = current_node;
			child_forward.st = apply_2(actFORWARD, current_node.st, mapa);
			child_forward.st.costeTotal += CalcularCoste(actFORWARD, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			if (explored.find(child_forward.st) == explored.end()) {
				child_forward.secuencia.push_back(actFORWARD);
				frontier.push(child_forward);
			}

			// Generar hijo actTURN_L
			nodeN2 child_turnl = current_node;
			child_turnl.st = apply_2(actTURN_L, current_node.st, mapa);
			child_turnl.st.costeTotal += CalcularCoste(actTURN_L, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			if (explored.find(child_turnl.st) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeN2 child_turnr = current_node;
			child_turnr.st = apply_2(actTURN_R, current_node.st, mapa);
			child_turnr.st.costeTotal += CalcularCoste(actTURN_R, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			if (explored.find(child_turnr.st) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push(child_turnr);
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.top();
			while (!frontier.empty() && explored.find(current_node.st) != explored.end()) {
				frontier.pop();
				if (!frontier.empty())
					current_node = frontier.top();
			}
		}
	}

	if(SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}

/////////////////////////////////// NIVEL 3 ////////////////////////////////////////

/**
 * Usamos chebyshev como heurística para este nivel.
*/
int Heuristica (int filaActual, int columnaActual, int filaDestino, int columnaDestino) {
    int dx = abs(filaDestino - filaActual);
    int dy = abs(columnaDestino - columnaActual);
    return max(dx, dy);
}

int CalcularCoste2(const Action &a, stateN3 &st, char tipocasilla) {
	int coste = 0;
	switch (a) 	{
		case actFORWARD:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_jugador) {
						coste = 100;
					} else {
						coste = 10;
					}
				break;
				case 'B':
					if (!st.zapatillas_jugador) {
						coste = 50;
					} else {
						coste = 15;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
				}
		break;
		case actTURN_L:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_jugador) {
						coste = 25;
					} else {
						coste = 5;
					}
				break;
				case 'B':
					if (!st.zapatillas_jugador) {
						coste = 5;
					} else {
						coste = 1;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
			}
		break;
		case actTURN_R:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_jugador) {
						coste = 25;
					} else {
						coste = 5;
					}
				break;
				case 'B':
					if (!st.zapatillas_jugador) {
						coste = 5;
					} else {
						coste = 1;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
			}
		break;
		case actSON_FORWARD:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_sonambulo) {
						coste = 100;
					} else {
						coste = 10;
					}
				break;
				case 'B':
					if (!st.zapatillas_sonambulo) {
						coste = 50;
					} else {
						coste = 15;
					}
				break;
				case 'T':
					coste = 2;
				break;
				default:
					coste = 1;
				break;
			}
		break;
		case actSON_TURN_SL:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_sonambulo) {
						coste = 7;
					} else {
						coste = 2;
					}
				break;
				case 'B':
					if (!st.zapatillas_sonambulo) {
						coste = 3;
					} else {
						coste = 1;
					}
				break;
				default:
					coste = 1;
				break;
			}
		break;
		case actSON_TURN_SR:
			switch (tipocasilla) {
				case 'A':
					if (!st.bikini_sonambulo) {
						coste = 7;
					} else {
						coste = 2;
					}
				break;
				case 'B':
					if (!st.zapatillas_sonambulo) {
						coste = 3;
					} else {
						coste = 1;
					}
				break;
				default:
					coste = 1;
				break;
			}
		break;
	}
	if (tipocasilla == '?') {
		coste = 0;
	}
	return coste;
}


/**
 * Devuelve el estado que se genera si el agente puede avanzar.
 * Si no puede avanzar, se devuelve como salida el mismo estado de entrada.
*/
stateN3 apply_3(const Action &a, stateN3 &st, const vector<vector<unsigned char>> &mapa) {
	char tipo_casilla_actual_jugador = mapa[st.jugador.f][st.jugador.c];
	if (tipo_casilla_actual_jugador == 'K') {
		st.bikini_jugador = true;
		st.zapatillas_jugador = false;
	}
	if (tipo_casilla_actual_jugador == 'D') {
		st.zapatillas_jugador = true;
		st.bikini_jugador = false;
	}
	char tipo_casilla_actual_sonambulo = mapa[st.sonambulo.f][st.sonambulo.c];
	if (tipo_casilla_actual_sonambulo == 'K') {
		st.bikini_sonambulo = true;
		st.zapatillas_sonambulo = false;
	}
	if (tipo_casilla_actual_sonambulo == 'D') {
		st.zapatillas_sonambulo = true;
		st.bikini_sonambulo = false;
	}

	stateN3 st_result = st;
	ubicacion sig_ubicacion;

	switch (a) {
		case actFORWARD:
			sig_ubicacion = NextCasilla(st.jugador);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.sonambulo.f && sig_ubicacion.c == st.sonambulo.c)) {
				st_result.jugador = sig_ubicacion;
			}
		break;
		case actTURN_L:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+6)%8);
		break;
		case actTURN_R:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+2)%8);
		break;
		case actSON_FORWARD:
			sig_ubicacion = NextCasilla(st.sonambulo);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.jugador.f && sig_ubicacion.c == st.jugador.c)) {
				st_result.sonambulo = sig_ubicacion;
			}
		break;
		case actSON_TURN_SL:
			st_result.sonambulo.brujula = static_cast<Orientacion>((st_result.sonambulo.brujula+7)%8);
		break;
		case actSON_TURN_SR:
			st_result.sonambulo.brujula = static_cast<Orientacion>((st_result.sonambulo.brujula+1)%8);
		break;
	}
	return st_result;
}

void ComportamientoJugador::VisualizaPlan3 (const stateN3 &st, const list<Action> &plan) {
	AnularMatriz(mapaConPlan);
	stateN3 cst = st;

	auto it = plan.begin();
	while (it != plan.end()) {
		switch (*it){
			case actFORWARD:
				cst.jugador = NextCasilla(cst.jugador);
				mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			break;
			case actTURN_L:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+6)%8);
			break;
			case actTURN_R:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+2)%8);
			break;
			case actSON_FORWARD:
				cst.sonambulo = NextCasilla(cst.sonambulo);
				mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
			case actSON_TURN_SL:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+7)%8);
			break;
			case actSON_TURN_SR:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+1)%8);
			break;
		}
		it++;
	}
}

/**
 * Comprueba si el sonámbulo está en el campo de visión del jugador.
*/
bool SonambuloAlaVista2(const stateN3 &st) {
	bool encontrado = false;
	bool pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8, pos9, pos10, pos11, pos12, pos13, pos14, pos15;
	switch (st.jugador.brujula) {
		case norte:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos7 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos11 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos13 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos14 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case noreste:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos5 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos8 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos10 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos11 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos14 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos15 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case este:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos6 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos11 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos12 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos14 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case sureste:
			pos1 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos4 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos9 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos11 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos14 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
		break;
		case sur:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos7 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos11 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos13 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos14 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case suroeste:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos5 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos8 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos10 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos11 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos14 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos15 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case oeste:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos6 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos11 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos12 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos14 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case noroeste:
			pos1 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos4 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos9 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos11 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos14 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
		break;
		default:
			return false; // Hay un test que pone orientación 8, entonces se inicializan variables pos mal.
			// Otra opcion es inicializar todas las variables pos, que quizá sería más correcto. En cualquier caso esta solución es valida.
		break;
	}
	if (pos1 || pos2 || pos3 || pos4 || pos5 || pos6 || pos7 || pos8 || pos9 || pos10 || pos11 || pos12 || pos13 || pos14 || pos15) {
		encontrado = true;
	}

	return encontrado;
}

/**
 * Solución óptima para el jugador y el sonánbulo (nivel 3).
*/
list<Action> DjikstraAmbos(const stateN3 &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeN3 current_node;
	current_node.st = inicio;
	priority_queue<nodeN3> frontier;
	set<stateN3> explored;
	list<Action> plan;
	bool SolutionFound = (current_node.st.sonambulo.f == final.f && current_node.st.sonambulo.c == final.c);
	bool encontrado = SonambuloAlaVista2(current_node.st);
	frontier.push(current_node);

	while (!frontier.empty() && !SolutionFound) {
		frontier.pop();
		explored.insert(current_node.st);
		encontrado = SonambuloAlaVista2(current_node.st);

		if (current_node.st.sonambulo.f == final.f && current_node.st.sonambulo.c == final.c) {
			SolutionFound = true;
		}

		if (!SolutionFound) {
			// Generar hijo actFORWARD
			nodeN3 child_forward = current_node;
			child_forward.st = apply_3(actFORWARD, current_node.st, mapa);
			child_forward.st.costeTotal += CalcularCoste2(actFORWARD, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_forward.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c);
			if (explored.find(child_forward.st) == explored.end()) {
				child_forward.secuencia.push_back(actFORWARD);
				frontier.push(child_forward);
			}
			// Generar hijo actTURN_L
			nodeN3 child_turnl = current_node;
			child_turnl.st = apply_3(actTURN_L, current_node.st, mapa);
			child_turnl.st.costeTotal += CalcularCoste2(actTURN_L, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_turnl.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c);
			if (explored.find(child_turnl.st) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeN3 child_turnr = current_node;
			child_turnr.st = apply_3(actTURN_R, current_node.st, mapa);
			child_turnr.st.costeTotal += CalcularCoste2(actTURN_R, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_turnr.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c);
			if (explored.find(child_turnr.st) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push(child_turnr);
			}

			if (encontrado) {
				// Generar hijo actSON_FORWARD
				nodeN3 child_SON_forward = current_node;
				child_SON_forward.st = apply_3(actSON_FORWARD, current_node.st, mapa);
				child_SON_forward.st.costeTotal += CalcularCoste2(actSON_FORWARD, current_node.st, mapa[current_node.st.sonambulo.f][current_node.st.sonambulo.c]);
				child_SON_forward.st.heuristica = Heuristica(child_SON_forward.st.sonambulo.f, child_SON_forward.st.sonambulo.c, final.f, final.c);
				if (explored.find(child_SON_forward.st) == explored.end()) {
					child_SON_forward.secuencia.push_back(actSON_FORWARD);
					frontier.push(child_SON_forward);
				}
				// Generar hijo actSON_TURN_SL
				nodeN3 child_turnsl = current_node;
				child_turnsl.st = apply_3(actSON_TURN_SL, current_node.st, mapa);
				child_turnsl.st.costeTotal += CalcularCoste2(actSON_TURN_SL, current_node.st, mapa[current_node.st.sonambulo.f][current_node.st.sonambulo.c]);
				child_turnsl.st.heuristica = Heuristica(child_turnsl.st.sonambulo.f, child_turnsl.st.sonambulo.c, final.f, final.c);
				if (explored.find(child_turnsl.st) == explored.end()) {
					child_turnsl.secuencia.push_back(actSON_TURN_SL);
					frontier.push(child_turnsl);
				}
				// Generar hijo actSON_TURN_SR
				nodeN3 child_turnsr = current_node;
				child_turnsr.st = apply_3(actSON_TURN_SR, current_node.st, mapa);
				child_turnsr.st.costeTotal += CalcularCoste2(actSON_TURN_SR, current_node.st, mapa[current_node.st.sonambulo.f][current_node.st.sonambulo.c]);
				child_turnsr.st.heuristica = Heuristica(child_turnsr.st.sonambulo.f, child_turnsr.st.sonambulo.c, final.f, final.c);
				if (explored.find(child_turnsr.st) == explored.end()) {
					child_turnsr.secuencia.push_back(actSON_TURN_SR);
					frontier.push(child_turnsr);
				}
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.top();
			while (!frontier.empty() && explored.find(current_node.st) != explored.end()) {
				frontier.pop();
				if (!frontier.empty())
					current_node = frontier.top();
			}
		}
	}

	if(SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}

/////////////////////////////////// NIVEL 4 ////////////////////////////////////////
/**
 * @brief Actualizamos la posición del agente en el mapaResultado según se va desplazando.
 * 
 * @param a 
 * @param girar_derecha 
 * @param lastAction 
 * @param st 
 */
/*void ActualizarMapaResultado(int & a, bool & girar_derecha, Action lastAction, stateN3 &st) {

	switch(lastAction) {
		case actFORWARD:
			switch(st.jugador.brujula) {
				case norte:
					st.jugador.f--;
				break;
				case noreste:
					st.sonambulo.f--;
					st.sonambulo.c++;
				break;
				case este:
					st.jugador.c++;
				break;
				case sureste:
					st.sonambulo.f++;
					st.sonambulo.c++;
				break;
				case sur:
					st.jugador.f++;
				break;
				case suroeste:
					st.sonambulo.f++;
					st.sonambulo.c--;
				break;
				case oeste:
					st.jugador.c--;
				break;
				case noroeste:
					st.sonambulo.f--;
					st.sonambulo.c--;
				break;
			}
		break;
		case actTURN_SL:
			a = st.brujula;
			a = (a+7)%8;
			st.brujula = static_cast<Orientacion>(a);
			girar_derecha = (rand()%2 ==0);
		break;
		case actTURN_SR:
			a = st.brujula;
			a = (a+1)%8;
			st.brujula = static_cast<Orientacion>(a);
			girar_derecha = (rand()%2 ==0);
		break;
		case actTURN_L:
			a = st.brujula;
			a = (a+5)%8;
			st.brujula = static_cast<Orientacion>(a);
		break;
		case actTURN_R:
			a = st.brujula;
			a = (a+3)%8;
			st.brujula = static_cast<Orientacion>(a);
		break;
	}

}*/

/**
 * @brief Pintamos en el mapaResultado lo que vamos viendo.
 * 
 * @param terreno 
 * @param st 
 * @param matriz 
 */
void PonerTerrenoEnMatriz(const vector<unsigned char> & terreno, const stateN3 &st, vector<vector<unsigned char>> &matriz) {

	int fila = st.jugador.f;
	int col = st.jugador.c;

	switch(st.jugador.brujula) {
		case norte:
			matriz[fila][col] = terreno[0];
			matriz[fila-1][col-1] = terreno[1];
			matriz[fila-1][col] = terreno[2];
			matriz[fila-1][col+1] = terreno[3];
			matriz[fila-2][col-2] = terreno[4];
			matriz[fila-2][col-1] = terreno[5];
			matriz[fila-2][col] = terreno[6];
			matriz[fila-2][col+1] = terreno[7];
			matriz[fila-2][col+2] = terreno[8];
			matriz[fila-3][col-3] = terreno[9];
			matriz[fila-3][col-2] = terreno[10];
			matriz[fila-3][col-1] = terreno[11];
			matriz[fila-3][col] = terreno[12];
			matriz[fila-3][col+1] = terreno[13];
			matriz[fila-3][col+2] = terreno[14];
			matriz[fila-3][col+3] = terreno[15];
		break;
		case noreste:
			matriz[fila][col] = terreno[0];
			matriz[fila-1][col] = terreno[1];
			matriz[fila-1][col+1] = terreno[2];
			matriz[fila][col+1] = terreno[3];
			matriz[fila-2][col] = terreno[4];
			matriz[fila-2][col+1] = terreno[5];
			matriz[fila-2][col+2] = terreno[6];
			matriz[fila-1][col+2] = terreno[7];
			matriz[fila][col+2] = terreno[8];
			matriz[fila-3][col] = terreno[9];
			matriz[fila-3][col+1] = terreno[10];
			matriz[fila-3][col+2] = terreno[11];
			matriz[fila-3][col+3] = terreno[12];
			matriz[fila-2][col+3] = terreno[13];
			matriz[fila-1][col+3] = terreno[14];
			matriz[fila][col+3] = terreno[15];
		break;
		case este:
			matriz[fila][col] = terreno[0];
			matriz[fila-1][col+1] = terreno[1];
			matriz[fila][col+1] = terreno[2];
			matriz[fila+1][col+1] = terreno[3];
			matriz[fila-2][col+2] = terreno[4];
			matriz[fila-1][col+2] = terreno[5];
			matriz[fila][col+2] = terreno[6];
			matriz[fila+1][col+2] = terreno[7];
			matriz[fila+2][col+2] = terreno[8];
			matriz[fila-3][col+3] = terreno[9];
			matriz[fila-2][col+3] = terreno[10];
			matriz[fila-1][col+3] = terreno[11];
			matriz[fila][col+3] = terreno[12];
			matriz[fila+1][col+3] = terreno[13];
			matriz[fila+2][col+3] = terreno[14];
			matriz[fila+3][col+3] = terreno[15];
		break;
		case sureste:
			matriz[fila][col] = terreno[0];
			matriz[fila][col+1] = terreno[1];
			matriz[fila+1][col+1] = terreno[2];
			matriz[fila+1][col] = terreno[3];
			matriz[fila][col+2] = terreno[4];
			matriz[fila+1][col+2] = terreno[5];
			matriz[fila+2][col+2] = terreno[6];
			matriz[fila+2][col+1] = terreno[7];
			matriz[fila+2][col] = terreno[8];
			matriz[fila][col+3] = terreno[9];
			matriz[fila+1][col+3] = terreno[10];
			matriz[fila+2][col+3] = terreno[11];
			matriz[fila+3][col+3] = terreno[12];
			matriz[fila+3][col+2] = terreno[13];
			matriz[fila+3][col+1] = terreno[14];
			matriz[fila+3][col] = terreno[15];
		break;
		case sur:
			matriz[fila][col] = terreno[0];
			matriz[fila+1][col+1] = terreno[1];
			matriz[fila+1][col] = terreno[2];
			matriz[fila+1][col-1] = terreno[3];
			matriz[fila+2][col+2] = terreno[4];
			matriz[fila+2][col+1] = terreno[5];
			matriz[fila+2][col] = terreno[6];
			matriz[fila+2][col-1] = terreno[7];
			matriz[fila+2][col-2] = terreno[8];
			matriz[fila+3][col+3] = terreno[9];
			matriz[fila+3][col+2] = terreno[10];
			matriz[fila+3][col+1] = terreno[11];
			matriz[fila+3][col] = terreno[12];
			matriz[fila+3][col-1] = terreno[13];
			matriz[fila+3][col-2] = terreno[14];
			matriz[fila+3][col-3] = terreno[15];
		break;
		case suroeste:
			matriz[fila][col] = terreno[0];
			matriz[fila+1][col] = terreno[1];
			matriz[fila+1][col-1] = terreno[2];
			matriz[fila][col-1] = terreno[3];
			matriz[fila+2][col] = terreno[4];
			matriz[fila+2][col-1] = terreno[5];
			matriz[fila+2][col-2] = terreno[6];
			matriz[fila+1][col-2] = terreno[7];
			matriz[fila][col-2] = terreno[8];
			matriz[fila+3][col] = terreno[9];
			matriz[fila+3][col-1] = terreno[10];
			matriz[fila+3][col-2] = terreno[11];
			matriz[fila+3][col-3] = terreno[12];
			matriz[fila+2][col-3] = terreno[13];
			matriz[fila+1][col-3] = terreno[14];
			matriz[fila][col-3] = terreno[15];
		break;
		case oeste:
			matriz[fila][col] = terreno[0];
			matriz[fila+1][col-1] = terreno[1];
			matriz[fila][col-1] = terreno[2];
			matriz[fila-1][col-1] = terreno[3];
			matriz[fila+2][col-2] = terreno[4];
			matriz[fila+1][col-2] = terreno[5];
			matriz[fila][col-2] = terreno[6];
			matriz[fila-1][col-2] = terreno[7];
			matriz[fila-2][col-2] = terreno[8];
			matriz[fila+3][col-3] = terreno[9];
			matriz[fila+2][col-3] = terreno[10];
			matriz[fila+1][col-3] = terreno[11];
			matriz[fila][col-3] = terreno[12];
			matriz[fila-1][col-3] = terreno[13];
			matriz[fila-2][col-3] = terreno[14];
			matriz[fila-3][col-3] = terreno[15];
		break;
		case noroeste:
			matriz[fila][col] = terreno[0];
			matriz[fila][col-1] = terreno[1];
			matriz[fila-1][col-1] = terreno[2];
			matriz[fila-1][col] = terreno[3];
			matriz[fila][col-2] = terreno[4];
			matriz[fila-1][col-2] = terreno[5];
			matriz[fila-2][col-2] = terreno[6];
			matriz[fila-2][col-1] = terreno[7];
			matriz[fila-2][col] = terreno[8];
			matriz[fila][col-3] = terreno[9];
			matriz[fila-1][col-3] = terreno[10];
			matriz[fila-2][col-3] = terreno[11];
			matriz[fila-3][col-3] = terreno[12];
			matriz[fila-3][col-2] = terreno[13];
			matriz[fila-3][col-1] = terreno[14];
			matriz[fila-3][col] = terreno[15];
		break;
	}
}

void PintarMurosIniciales(vector<vector<unsigned char>> &mapa) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < mapa.size(); j++) {
			mapa[i][j] = 'P';
		}
	}
	for (int i = mapa.size()-3; i < mapa.size(); i++) {
		for (int j = 0; j < mapa.size(); j++) {
			mapa[i][j] = 'P';
		}
	}
	for (int i = 3; i < mapa.size()-3; i++) {
		for (int j = 0; j < 3; j++) {
			mapa[i][j] = 'P';
		}
	}
	for (int i = 3; i < mapa.size()-3; i++) {
		for (int j = mapa.size()-3; j < mapa.size(); j++) {
			mapa[i][j] = 'P';
		}
	}
}

/**
 * @brief Utilizamos esta heuristica para mover al jugador.
 * 
 * @param x1 
 * @param y1 
 * @param x2 
 * @param y2 
 * @return int 
 */
int Manhattan(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    return dx + dy;
}

/**
 * @brief Algoritmo para encontrar al sonámbulo.
 * 
 * @param inicio 
 * @param mapa 
 * @param sensores 
 * @return list<Action> 
 */
list<Action> PrimerEncuentroSonambulo(const stateN3 &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa, const Sensores &sensores) {
	nodeN3 current_node;
	current_node.st = inicio;
	priority_queue<nodeN3> frontier;
	set<stateN3> explored;
	list<Action> plan;
	bool encontrado = SonambuloAlaVista2(current_node.st);
	frontier.push(current_node);

	while (!frontier.empty() && !encontrado) {
		frontier.pop();
		explored.insert(current_node.st);
		encontrado = SonambuloAlaVista2(current_node.st);

		if (!encontrado) {
			// Generar hijo actFORWARD
			nodeN3 child_forward = current_node;
			child_forward.st = apply_3(actFORWARD, current_node.st, mapa);
			child_forward.st.costeTotal += CalcularCoste2(actFORWARD, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_forward.st.heuristica = Manhattan(child_forward.st.jugador.f, child_forward.st.jugador.c, child_forward.st.sonambulo.f, child_forward.st.sonambulo.c);
			if (explored.find(child_forward.st) == explored.end()) {
				child_forward.secuencia.push_back(actFORWARD);
				frontier.push(child_forward);
			}
			// Generar hijo actTURN_L
			nodeN3 child_turnl = current_node;
			child_turnl.st = apply_3(actTURN_L, current_node.st, mapa);
			child_turnl.st.costeTotal += CalcularCoste2(actTURN_L, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_turnl.st.heuristica = Manhattan(child_forward.st.jugador.f, child_forward.st.jugador.c, child_forward.st.sonambulo.f, child_forward.st.sonambulo.c);
			if (explored.find(child_turnl.st) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeN3 child_turnr = current_node;
			child_turnr.st = apply_3(actTURN_R, current_node.st, mapa);
			child_turnr.st.costeTotal += CalcularCoste2(actTURN_R, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_turnr.st.heuristica = Manhattan(child_forward.st.jugador.f, child_forward.st.jugador.c, child_forward.st.sonambulo.f, child_forward.st.sonambulo.c); 
			if (explored.find(child_turnr.st) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push(child_turnr);
			}
		}

		if (!encontrado && !frontier.empty()) {
			current_node = frontier.top();
			while (!frontier.empty() && explored.find(current_node.st) != explored.end()) {
				frontier.pop();
				if (!frontier.empty())
					current_node = frontier.top();
			}
		}
	}

	if (encontrado) {
		plan = current_node.secuencia;
	}

	return plan;
}

/**
 * @brief Solución óptima para el jugador y el sonánbulo (nivel 4).
 * 
 * @param inicio 
 * @param mapa 
 * @param sensores 
 * @return list<Action> 
 */
list<Action> Reto(const stateN3 &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeN3 current_node;
	current_node.st = inicio;
	priority_queue<nodeN3> frontier;
	set<stateN3> explored;
	list<Action> plan;
	bool SolutionFound = ((current_node.st.sonambulo.f == final.f && current_node.st.sonambulo.c == final.c) || (current_node.st.jugador.f == final.f && current_node.st.jugador.c == final.c));
	bool encontrado = SonambuloAlaVista2(current_node.st);
	frontier.push(current_node);

	while (!frontier.empty() && !SolutionFound) {
		frontier.pop();
		explored.insert(current_node.st);
		encontrado = SonambuloAlaVista2(current_node.st);

		// Como nos dan muchos más puntos por llegar el sonámbulo, debería de crear una heurística para calcular
		// si nos interesa más ir al destino con jugador o con el sonámbulo, según la distancia de cada uno al objetivo.
		if ((current_node.st.sonambulo.f == final.f && current_node.st.sonambulo.c == final.c) || (current_node.st.jugador.f == final.f && current_node.st.jugador.c == final.c)) {
			SolutionFound = true;
		}

		if (!SolutionFound) {
			// Generar hijo actFORWARD
			nodeN3 child_forward = current_node;
			child_forward.st = apply_3(actFORWARD, current_node.st, mapa);
			child_forward.st.costeTotal += CalcularCoste2(actFORWARD, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_forward.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c); 
			if (explored.find(child_forward.st) == explored.end()) {
				child_forward.secuencia.push_back(actFORWARD);
				frontier.push(child_forward);
			}
			// Generar hijo actTURN_L
			nodeN3 child_turnl = current_node;
			child_turnl.st = apply_3(actTURN_L, current_node.st, mapa);
			child_turnl.st.costeTotal += CalcularCoste2(actTURN_L, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_turnl.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c);
			if (explored.find(child_turnl.st) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeN3 child_turnr = current_node;
			child_turnr.st = apply_3(actTURN_R, current_node.st, mapa);
			child_turnr.st.costeTotal += CalcularCoste2(actTURN_R, current_node.st, mapa[current_node.st.jugador.f][current_node.st.jugador.c]);
			child_turnr.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c); 
			if (explored.find(child_turnr.st) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push(child_turnr);
			}

			if (encontrado) {
				// Generar hijo actSON_FORWARD
				nodeN3 child_SON_forward = current_node;
				child_SON_forward.st = apply_3(actSON_FORWARD, current_node.st, mapa);
				child_SON_forward.st.costeTotal += CalcularCoste2(actSON_FORWARD, current_node.st, mapa[current_node.st.sonambulo.f][current_node.st.sonambulo.c]);
				child_SON_forward.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c); 
				if (explored.find(child_SON_forward.st) == explored.end()) {
					child_SON_forward.secuencia.push_back(actSON_FORWARD);
					frontier.push(child_SON_forward);
				}
				// Generar hijo actSON_TURN_SL
				nodeN3 child_turnsl = current_node;
				child_turnsl.st = apply_3(actSON_TURN_SL, current_node.st, mapa);
				child_turnsl.st.costeTotal += CalcularCoste2(actSON_TURN_SL, current_node.st, mapa[current_node.st.sonambulo.f][current_node.st.sonambulo.c]);
				child_turnsl.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c); 
				if (explored.find(child_turnsl.st) == explored.end()) {
					child_turnsl.secuencia.push_back(actSON_TURN_SL);
					frontier.push(child_turnsl);
				}
				// Generar hijo actSON_TURN_SR
				nodeN3 child_turnsr = current_node;
				child_turnsr.st = apply_3(actSON_TURN_SR, current_node.st, mapa);
				child_turnsr.st.costeTotal += CalcularCoste2(actSON_TURN_SR, current_node.st, mapa[current_node.st.sonambulo.f][current_node.st.sonambulo.c]);
				child_turnsr.st.heuristica = Heuristica(current_node.st.sonambulo.f, current_node.st.sonambulo.c, final.f, final.c); 
				if (explored.find(child_turnsr.st) == explored.end()) {
					child_turnsr.secuencia.push_back(actSON_TURN_SR);
					frontier.push(child_turnsr);
				}
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.top();
			while (!frontier.empty() && explored.find(current_node.st) != explored.end()) {
				frontier.pop();
				if (!frontier.empty())
					current_node = frontier.top();
			}
		}
	}

	if(SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}

// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores) {
	Action accion = actIDLE;

	// Incluir aquí el comportamiento del agente jugador
	if (sensores.nivel != 4) {
		if (!hayPlan) {
			cout << "Calculando un nuevo plan\n";
			goal.f = sensores.destinoF;
			goal.c = sensores.destinoC;

			switch (sensores.nivel){
				case 0:
					c_state.jugador.f = sensores.posF;
					c_state.jugador.c = sensores.posC;
					c_state.jugador.brujula = sensores.sentido;
					c_state.sonambulo.f = sensores.SONposF;
					c_state.sonambulo.c = sensores.SONposC;
					c_state.sonambulo.brujula = sensores.SONsentido;
					plan = AnchuraSoloJugador(c_state, goal, mapaResultado);
					if (plan.size() > 0) {
						VisualizaPlan0(c_state, plan);
						hayPlan = true;
					}
 				break;
				case 1:
					c_state_1.jugador.f = sensores.posF;
					c_state_1.jugador.c = sensores.posC;
					c_state_1.jugador.brujula = sensores.sentido;
					c_state_1.sonambulo.f = sensores.SONposF;
					c_state_1.sonambulo.c = sensores.SONposC;
					c_state_1.sonambulo.brujula = sensores.SONsentido;
					plan = AnchuraAmbos(sensores, c_state_1, goal, mapaResultado);
					if (plan.size() > 0) {
						VisualizaPlan1(c_state_1, plan);
						hayPlan = true;
					}
				break;
				case 2:
					c_state_2.jugador.f = sensores.posF;
					c_state_2.jugador.c = sensores.posC;
					c_state_2.jugador.brujula = sensores.sentido;
					c_state_2.sonambulo.f = sensores.SONposF;
					c_state_2.sonambulo.c = sensores.SONposC;
					c_state_2.sonambulo.brujula = sensores.SONsentido;
					c_state_2.bikini_jugador = false;
					c_state_2.zapatillas_jugador = false;
					c_state_2.costeTotal = 0;
					plan = DjikstraJugador(c_state_2, goal, mapaResultado);
					if (plan.size() > 0) {
						VisualizaPlan2(c_state_2, plan);
						hayPlan = true;
					}
				break;
				case 3:
					c_state_3.jugador.f = sensores.posF;
					c_state_3.jugador.c = sensores.posC;
					c_state_3.jugador.brujula = sensores.sentido;
					c_state_3.sonambulo.f = sensores.SONposF;
					c_state_3.sonambulo.c = sensores.SONposC;
					c_state_3.sonambulo.brujula = sensores.SONsentido;
					c_state_3.bikini_jugador = false;
					c_state_3.zapatillas_jugador = false;
					c_state_3.costeTotal = 0;
					plan = DjikstraAmbos(c_state_3, goal, mapaResultado);
					if (plan.size() > 0) {
						VisualizaPlan3(c_state_3, plan);
						hayPlan = true;
					}
				break;
			}

		}
		if (hayPlan && plan.size() > 0) {
			cout << "Ejecutando la siguiente acción del plan\n";
			accion = plan.front();
			plan.pop_front();
		}
		if (plan.size() == 0) {
			cout << "Se completó el plan\n";
			hayPlan = false;
		}
	} else {
		// AQUI NIVEL 4
		if (!murosIniciales) {
			PintarMurosIniciales(mapaResultado);
			murosIniciales = true;
		}

		if (last_action == actWHEREIS) {
			c_state_4.jugador.f = sensores.posF;
			c_state_4.jugador.c = sensores.posC;
			c_state_4.jugador.brujula = sensores.sentido;
			c_state_4.sonambulo.f = sensores.SONposF;
			c_state_4.sonambulo.c = sensores.SONposC;
			c_state_4.sonambulo.brujula = sensores.SONsentido;
		}

		if (ubicado) {
			c_state_4 = apply_3(last_action, c_state_4, mapaResultado);
			PonerTerrenoEnMatriz(sensores.terreno, c_state_4, mapaResultado);
		}
		
		if (!ubicado) {
			ubicado = true;
			accion = actWHEREIS;
		} else if (!hayPlan && !primerEncuentroSonambulo) {
			plan = PrimerEncuentroSonambulo(c_state_4, goal, mapaResultado, sensores);
			hayPlan = true;
		} else if (!hayPlan) {
			plan = Reto(c_state_4, goal, mapaResultado);
			hayPlan = true;
			cout << "GGGGG" << endl;
		}

		// Si el sonmabulo está a la vista en algún momento --> El primer encuentro debe haberse producido ya.
		if (SonambuloAlaVista2(c_state_4)) {
			primerEncuentroSonambulo = true;
		}

		if (plan.size() > 0) {
			VisualizaPlan3(c_state_4, plan);
			hayPlan = true;
		}

		if (hayPlan && plan.size() > 0) {
			cout << "Ejecutando la siguiente acción del plan\n";
			accion = plan.front();
			plan.pop_front();
		}
		if (plan.size() == 0 && hayPlan) {
			cout << "Se completó el plan\n";
			hayPlan = false;
		}
		
		// Por si el plan por el mapa desconocido atraviesa un muro, precipicio o intenta ocupar una casilla ocupada.
		if (((sensores.terreno[2] == 'P' || sensores.terreno[2] == 'M') || sensores.superficie[2] != '_') && accion == actFORWARD) {
			accion = actIDLE;
			plan.clear();
			hayPlan = false;
		}

		// Habrá que añadir nuevos condicionales para que si se va a meter
		// por terreno que gaste mucha bateria sin objetos intente recalcular con el A* con el mapa que tenemos 
		// en ese momento disponible.
		
		// NOTA: Podría pensar alguna forma de comprobar si hay desplazamiento??????
		if (sensores.colision) {
			hayPlan = false; // Ya no sirve el plan anterior.
			ubicado = false;
			plan.clear();
			accion = actIDLE; // Metemos acción en su lugar.
		}

	}
	last_action = accion;
	return accion;
}


int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}