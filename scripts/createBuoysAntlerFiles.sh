#!/bin/bash

TIME_WARP=1
NB_BUOY=2

while :
do
  case $1 in
  -h | --help | -\?)
    printf "%s [nb_buoy] [time_warp] \n" $0
    printf "  --nb_buoy=value : default is $NB_BUOY, \n\t should be in [2 ; 4], \n\t else write meta_(NB_BUOY)buoys.moos \n\t(shortcuts: -nb) \n"
    printf "  --time_warp=value : default is $TIME_WARP \n\t(shortcuts: -t) \n"
    printf "  --help \n\t(shortcuts: -h, -?) \n" 
    exit 0
    ;;
  -nb=* | --nb_buoy=*)
    NB_BUOY=${1#*=}
    printf "\tNew parameter NB_BUOY: %d\n" $NB_BUOY
    shift
    ;;
  -t=* | --time_warp=*)
    TIME_WARP=${2#*=}
    printf "\tNew parameter TIME_WARP: %d\n" $TIME_WARP
    shift
    ;;
  --) # End of all options
    shift
    break
    ;;
  -*)
    echo "WARN: Unknown option (ignored): $1" >&2
    shift
    ;;
  *)  # no more options. Stop while loop
    break
    ;;
  esac
done

printf "\n"

printf "Configuring MOOS files.. \t\t"

for ((i=1; i<=$NB_BUOY; i++)); do
    NAME=AUV$i
    SERVER_HOST=localhost
    SERVER_PORT=9000
    PARAMETERS="
      NAME=$NAME \
      SERVER_HOST=$SERVER_HOST \
      SERVER_PORT=$SERVER_PORT \
      TIME_WARP=$TIME_WARP "
  nsplug meta_$[NB_BUOY]buoysNoConfig.moos targ_$[NB_BUOY]buoys_AUV$i.moos -f ${PARAMETERS}
done



printf "done\n" ; sleep 1

