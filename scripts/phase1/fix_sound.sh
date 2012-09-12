#!/bin/bash
amixer -c 0 sset 'Analog Right AUXR' nocap
amixer -c 0 sset 'Analog Left AUXL' nocap
amixer -c 0 sset 'Analog Left Main Mic' cap
amixer -c 0 sset 'Analog Right Sub Mic' cap
