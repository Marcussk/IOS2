
# testuje spravnou sekvenci cislovani akci a format vystupu (pouze syntax)
# bez zaruky, muze obsahovat chyby

outf="rivercrossing.out"
tr -d " " < ${outf} > $$

# test cislovani akci
# tiskne radky s chybnym cislovanim
cat $$ | awk -F":" ' { c++; if (c != $1) print NR, " => ", $1, " : chyba v cislovani akci"; } '


# kontrola sytaxe vystupu
# vytiskne radky, ktere neodpovidaji formatu vytupu
echo "=== radky, ktere neodpovidaji formatu vystupu"

cat $$ | sed '/^[1-9][0-9]*:hacker:[1-9][0-9]*:started$/d
/^[1-9][0-9]*:hacker:[1-9][0-9]*:waitingforboarding:[0-9][0-9]*:[0-9][0-9]*$/d
/^[1-9][0-9]*:hacker:[1-9][0-9]*:boarding:[0-9][0-9]*:[0-9][0-9]*$/d
/^[1-9][0-9]*:hacker:[1-9][0-9]*:landing:[0-9][0-9]*:[0-9][0-9]*$/d
/^[1-9][0-9]*:hacker:[1-9][0-9]*:member$/d
/^[1-9][0-9]*:hacker:[1-9][0-9]*:captain$/d
/^[1-9][0-9]*:hacker:[1-9][0-9]*:finished$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:started$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:waitingforboarding:[0-9][0-9]*:[0-9][0-9]*$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:boarding:[0-9][0-9]*:[0-9][0-9]*$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:landing:[0-9][0-9]*:[0-9][0-9]*$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:member$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:captain$/d
/^[1-9][0-9]*:serf:[1-9][0-9]*:finished$/d
'


# kontrola chybejicich vystupu
# tiskne informaci, ktery text ve vystupu chybi
echo "=== chybejici vystupy"
cat $$ | grep '^[1-9][0-9]*:hacker:[1-9][0-9]*:started$' >/dev/null || echo "hacker started"
cat $$ | grep '^[1-9][0-9]*:hacker:[1-9][0-9]*:waitingforboarding:[0-9][0-9]*:[0-9][0-9]*$' >/dev/null || echo "hacker waiting"
cat $$ | grep '^[1-9][0-9]*:hacker:[1-9][0-9]*:boarding:[0-9][0-9]*:[0-9][0-9]*$' >/dev/null || echo "hacker boarding"
cat $$ | grep '^[1-9][0-9]*:hacker:[1-9][0-9]*:landing:[0-9][0-9]*:[0-9][0-9]*$' >/dev/null || echo "hacker landing"
cat $$ | grep '^[1-9][0-9]*:hacker:[1-9][0-9]*:member$' >/dev/null || echo "hacker member"
cat $$ | grep '^[1-9][0-9]*:hacker:[1-9][0-9]*:finished$' >/dev/null || echo "hacker finished"
cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:started$' >/dev/null || echo "serf started"
cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:waitingforboarding:[0-9][0-9]*:[0-9][0-9]*$' >/dev/null || echo "serf waiting"
cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:boarding:[0-9][0-9]*:[0-9][0-9]*$' >/dev/null || echo "serf boarding"
cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:landing:[0-9][0-9]*:[0-9][0-9]*$' >/dev/null || echo "serf landing"
cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:member$' >/dev/null || echo "serf member"
#cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:captain$' >/dev/null || echo "serf captain"
cat $$ | grep '^[1-9][0-9]*:serf:[1-9][0-9]*:finished$' >/dev/null || echo "serf finished"

cat $$ | grep '^[1-9][0-9]*:[^:]*:[1-9][0-9]*:captain$' >/dev/null || echo "XXX captain"

rm $$
