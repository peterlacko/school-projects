#!/bin/bash

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
################################################################################
#
#       Filename: runtests.sh
#    Description: projekt #1 do kurzu IOS: praca so sadou regresnych testov
#        Created: 10.3.2012
#         Author: Peter Lacko, xlacko06@stud.fit.vutbr.cz
#	
################################################################################
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

# navratove kody
ALL_OK=0
OPP_ERR=1
WRONG_PARAMS=2

TEST_DIR=
REGEX=
found=0 # indikace lubovolneho najdeneho parametru
EXIT_STATUS=0

# promenne pre uchovanie informacii o parametroch
VAL=0 	# validate tree
RTE=0	# run tests
REP=0	# report results
SYN=0	# synchronize expected results
CLE=0	# clear generated files

################################################################################
# vytiskne napovedu a ukonci sa s chybovym kodom 2=wRONG_PARAMS
################################################################################
print_help () 
{
   printf "Usage: $0 [-vtrsc] TEST_DIR [REGEX]

   -v  validate tree
   -t  run tests
   -r  report results
   -s  synchronize expected results
   -c  clear generated files

   It is mandatory to supply at least one option.\n" >> /dev/stderr
   exit 2
}

# test na pocet prametrov
if [ "$#" -lt 2 -o "$#" -gt 7 ]; then
   print_help
fi

# spracovanie parametrov
while getopts vtrsc name
do
   case $name in
      v) VAL=1
	 found=1;;
      t) RTE=1
	 found=1;;
      r) REP=1
	 found=1;;
      s) SYN=1
	 found=1;;
      c) CLE=1
	 found=1;;
      ?) print_help;;
   esac
done

if [ "$found" -eq 0 ]; then
   print_help;
fi

let "OPTIND=$OPTIND-1"
shift $OPTIND

if [ "$#" -eq 0 -o "$#" -gt 2 ]; then # kontrola poctu parametrov
   print_help
fi

if [ ! -z "$1" ]; then 		# test na pritomnost parametru TEST_DIR
   TEST_DIR=$1			# test existencie adresara TEST_DIR
   if [ ! -d "$TEST_DIR" ]; then
      print_help
   fi
else
   print_help
fi

# shift $OPTIND
if [ ! -z "$2" ]; then 		# test na pritomnost parametru REGEX
   REGEX="$2"
else
   REGEX=".*"
fi

pushd "$TEST_DIR" 2&> /dev/null

################################################################################
# operace -v
################################################################################
if [ $VAL -eq 1 ]; then
   while read ad;
   do
      # kontrola na pritomnost suborov pri adresaroch
      if [ -n "$(find "$ad" -maxdepth 1 -mindepth 1 -type d|head -c 1)" \
	 -a -n "$(find "$ad" -maxdepth 1 -mindepth 1 -type f |head -c 1)" ]; then
      printf "$0: directories and other files mixed in $ad\n" >> /dev/stderr
      EXIT_STATUS=$OPP_ERR
   fi

   #kontrola pritomnosti symbolickych odkazov
   if [ -n "$(find "$ad" -maxdepth 1 -mindepth 1 -type l |head -c 1)" ]; then
      printf "$0: symbolic link(s) found in $ad\n" >> /dev/stderr
      EXIT_STATUS=$OPP_ERR
   fi

   # kontrola pritomnosti viacnasobnych pevnych odkzaov
   if [ -n "$(find "$ad" -maxdepth 1 -mindepth 1 -links +1 -type f |head -c 1)" ]; then
      printf "$0: multiple hard link file(s) found in $ad\n" >> /dev/stderr
      EXIT_STATUS=$OPP_ERR
   fi

   # test existencie suborov cmd-given a ich pristupovych prav
   if [ -z "$(find "$ad" -maxdepth 1 -mindepth 1 -type d)" ]; then
      if [ -e "${ad}/cmd-given" ]; then
	 if [ ! -x "${ad}/cmd-given" ]; then
	    printf "$0: no execute permissions on file $ad/cmd-given\n" >> /dev/stderr
	    EXIT_STATUS=$OPP_ERR
	 fi
      else
	 printf "$0: file \"cmd-given\" missing in $ad\n" >> /dev/stderr
	 EXIT_STATUS=$OPP_ERR
      fi
   fi

   # kontrola suborov stdin-given pro cteni
   if [ -e "${ad}/stdin-given" -a ! -r "${ad}/stdin-given" ]; then
      printf "$0: no read permissions on file $ad/stdin-given\n" >> /dev/stderr
      EXIT_STATUS=$OPP_ERR
   fi

   # test suborov {stdout,stderr,status}-{expected,captured,delta} na zapis
   while read testfile;
   do
      if [ ! -w "$testfile" ]; then
	 printf "$0: no write permissions on file $testfile\n" >> /dev/stderr
	 EXIT_STATUS=$OPP_ERR
      fi
   done< <(find "$ad" -maxdepth 1 -type f|grep -E "/((stdout)|(stderr)|(status))-((expected)|(captured)|(delta))$");

   # test suborov obsahujucich cislo
   while read testfile;
   do
      if [ "$(grep -c -m 2 "\$" "$testfile")" -ne 1 ]; then
	 printf "$0: extra newline or empty file $testfile\n" >> /dev/stderr
	 EXIT_STATUS=$OPP_ERR
      fi
      head -n 1 "$testfile"|grep -E "^[[:digit:]]+$"|grep -E -q -v $'\r'
      if [ "$?" -ne 0 -o -z "$(od -x "$testfile" |grep -iE -m 1 "0a")" ]; then
	 printf "$0: not a number in $testfile\n" >> /dev/stderr
	 EXIT_STATUS=$OPP_ERR
      fi
   done< <(find "$ad" -maxdepth 1 -type f|grep -E "/status-((expected)|(captured))$");

   # kontrola neznamych suborov
   if [ -n "$(find "$ad" -maxdepth 1 -mindepth 1 -type f| grep -E -v "/(cmd-given)|(stdin-given)|(((stdout)|(stderr)|(status))-((expected)|(captured)|(delta)))$")" ]; then
      printf "$0: unknown file(s) in $ad\n" >> /dev/stderr
      EXIT_STATUS=$OPP_ERR
   fi
done< <(find . -type d|grep -E "$REGEX"|sort);
fi

################################################################################
# operace -t / run tests
################################################################################
if [ $RTE -eq 1 ]; then
   while read curr_dir;
   do
      cd "$curr_dir" &> /dev/null
      if [ "$?" -ne 0 ]; then
	 EXIT_STATUS=$OPP_ERR
	 continue
      fi
      if [ -e "stdin-given" ]; then
	 ./cmd-given <stdin-given >stdout-captured 2>stderr-captured
      else
	 ./cmd-given </dev/null >stdout-captured 2>stderr-captured
      fi
      echo $? > status-captured

      # ziskanie rozdielov pomocou diff-u
      diff -up status-expected status-captured >status-delta 2> /dev/null
      DRV[0]=$?
      diff -up stdout-expected stdout-captured >stdout-delta 2> /dev/null
      DRV[1]=$?
      diff -up stderr-expected stderr-captured >stderr-delta 2> /dev/null
      DRV[2]=$?

      # ak sa niektory z diffov skoncil neuspechom, EXIT_STATUS=1
      if [ ${DRV[0]} -ne 0 -o ${DRV[1]} -ne 0 -o ${DRV[2]} -ne 0 ]; then
	 EXIT_STATUS=$OPP_ERR
      fi

      # odstraneni './' z nazvu adresara
      TEST="$(echo "$curr_dir" | sed 's/^\.\///' )"

      if [ ! -s stdout-delta -a ! -s stderr-delta -a ! -s status-delta ]; then
	 RESULT="OK"
      else
	 RESULT="FAILED"
	 EXIT_STATUS=$OPP_ERR
      fi
      if [ -t 2 ]; then
	 if [ "$RESULT" = "FAILED" ];then
	    RESULT="\033[1;31m$RESULT\033[0m"
	 else
	    RESULT="\033[1;32m$RESULT\033[0m"
	 fi
      fi
      printf "$TEST: $RESULT\n" >> /dev/stderr
      cd - &> /dev/null;
   done < <(find . -type f -name cmd-given -exec dirname {} \; |sort|grep -E "$REGEX");
fi

################################################################################
# operace -r / report results
################################################################################
if [ $REP -eq 1 ]; then
   while read curr_dir;
   do
      # ziskanie rozdielov pomocou diff-u
      diff -up "$curr_dir/status-expected" "$curr_dir/status-captured" >"$curr_dir/status-delta" 2> /dev/null
      DRV[0]=$?
      diff -up "$curr_dir/stdout-expected" "$curr_dir/stdout-captured" >"$curr_dir/stdout-delta" 2> /dev/null
      DRV[1]=$?
      diff -up "$curr_dir/stderr-expected" "$curr_dir/stderr-captured" >"$curr_dir/stderr-delta" 2> /dev/null
      DRV[2]=$?

      # ak sa niektory z diffov skoncil neuspechom, EXIT_STATUS=1
      if [ ${DRV[0]} -ne 0 -o ${DRV[1]} -ne 0 -o ${DRV[2]} -ne 0 ]; then
	 EXIT_STATUS=$OPP_ERR
      fi

      # odstraneni './' z nazvu adresara
      TEST="$(echo "$curr_dir" | sed 's/^\.\///' )"

      # ak maju subory *-delta nulovu dlzku, RESULT=OK
      if [ ! -s "$curr_dir/stdout-delta" -a ! -s "$curr_dir/stderr-delta" -a ! -s "$curr_dir/status-delta" ]; then
	 RESULT="OK"
      else
	 RESULT="FAILED"
	 EXIT_STATUS=$OPP_ERR
      fi
      if [ -t 1 ]; then
	 if [ "$RESULT" = "FAILED" ];then
	    RESULT="\033[1;31m$RESULT\033[0m"
	 else
	    RESULT="\033[1;32m$RESULT\033[0m"
	 fi
      fi
      printf "$TEST: $RESULT\n"; >> /dev/stdout
   done< <(find . -type f -name cmd-given -exec dirname {} \; |sort|grep -E "$REGEX");
fi

################################################################################
# operace -s / synchronize expected results
################################################################################
if [ $SYN -eq 1 ]; then
   while read old_name;
   do
      if [ "$(echo "$(dirname $old_name)" | grep -E -c "$REGEX")" -ne 0 ]; then
	 new_name="$(echo "$old_name" | sed -r 's/((stdout)|(stderr)|(status))-captured$/\1-expected/' )"
	 mv -f "$old_name" "$new_name" &> /dev/null;
      fi;
   done < <(find . -type f -name *captured |sort);
fi


################################################################################
# operace -c / clean generated files
################################################################################
if [ $CLE -eq 1 ]; then
   while read "rm_file";
   do
      if [ "$(echo "$(dirname "$rm_file")" | grep -E -c "$REGEX")" -ne 0 ]; then
	 rm -f "$rm_file" &> /dev/null;
      fi;
   done < <(find . -type f |sort| grep -E "((stdout)|(stderr)|(status))-((captured)|(delta))$");
fi
pushd +1 2&> /dev/null
popd 2&> /dev/null

exit $EXIT_STATUS
