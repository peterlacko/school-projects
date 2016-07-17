#!/usr/bin/env python3

#DKA:xlacko06
import sys
import getopt
import re

exit_codes = {
    "ALL_OK": 0,
    "PARAMS_ERROR": 1,
    "INPUT_ERROR": 2,
    "OUTPUT_ERROR": 3,
    "SYN_ERROR": 40,
    "SEM_ERROR": 41
}


################################################################################
# definice funkci
################################################################################

def print_help():
    """Funkce vytiske napovedu a ukonci program s navratovym kodem 0"""
    print (
        "pouziti: dka.py [parametry]\n\
        --help\n\
            vytiske tuto napovedu\n\
        --input=filename\n\
            zadany vstupni textovy soubor s popisem konecneho automatu\n\
        --output=filename\n\
            zadany vystupni soubor\n\
        -e ,--no-epsilon-rules\n\
            odstraneni epsilon pravidel\n\
        -d --determinization\n\
            provede determinizaci bez generovani nedostupnych stavu\n\
        -i --case-insensitive\n\
            nebude bran ohled na velikost znaku pri porovnavani symbolu ci\
 znaku\n")
    sys.exit(exit_codes["ALL_OK"])


def print_fsm(states, alphabet, rules, start_state, fstates):
    """Vytiskne vysledny automat v normalizovanej forme"""
    print('(',sep='',file=outfile)

    # tisk stavov
    slist = sorted(states) 
    print('{',sep='',end='', file=outfile)
    for item in slist:
        if item == slist[-1]:
            print(item, sep='', end='', file=outfile)
        else:
            print(item, ', ', sep='', end='', file=outfile)
    print('},', sep='', file=outfile)

    # tisk symbolov vstupnej abecedy
    slist = sorted(alphabet)
    print('{',sep='',end='',file=outfile)
    for item in slist:
        if item == slist[-1]:
            print('\'', item, '\'', sep='', end='', file=outfile)
        else:
            print('\'', item, '\'', ', ', sep='', end='', file=outfile)
    print('},', sep='', file=outfile)

    # tisk previdel
    slist = sorted(rules)
    print('{',sep='', file=outfile)
    for item in slist:
        if item == slist[-1]:
            print(item, sep='', file=outfile)
        else:
            print(item, sep='', end=',\n', file=outfile)
    print('},', sep='', file=outfile)

    # startovaci stav
    print(start_state, sep='', end=',\n', file=outfile)

    # koncove stavy
    slist = sorted(fstates) 
    print('{',sep='',end='',file=outfile)
    for item in slist:
        if item == slist[-1]:
            print(item, sep='', end='', file=outfile)
        else:
            print(item, ', ', sep='', end='', file=outfile)
    print('}\n)', sep='', end='', file=outfile)

def gen_state(states):
    """Zlucuje stavy v mnozine states """
    fin_state = ''
    sstates = sorted(states)
    for state in sstates:
        if state == sstates[0]:
            fin_state = state
        else:
            fin_state = fin_state + '_' + state
    return fin_state

################################################################################
################################################################################

################################################################################
# definice tried
################################################################################
class Rules:
    """Trieda obsahuje metody pre pracu s mnozinami, zmenu identifikatorov 
    na male pismena atd"""
    def __init__(self, source, symbol, dest):
        # ak su zdroj alebo ciel mnoziny stavov, vytvori z nich jeden stav
        if type(source) == set or type(source) == frozenset:
            self.source = gen_state(source)
        else:
            self.source = source

        if type(dest) == set or type(dest) == frozenset:
            self.dest = gen_state(dest)
        else:
            self.dest = dest

        # pri epsilon prechode sa vstupny symbol nahradi ''
        if symbol == None:
            self.symbol = ''
        else:
            self.symbol = symbol
        self.rule = self.source + ' \'' + self.symbol + '\' -> ' + self.dest

    def __repr__(self):
        return self.rule

    def __str__(self):
        return self.rule

    def __hash__(self):
        return hash(self.rule)

    def __lt__(self, other):
        return self.rule < other.rule

    def __le__(self, other):
        return self.rule <= other.rule

    def __eq__(self, other):
        return self.rule == other.rule

    def __ne__(self, other):
        return self.rule != other.rule

    def __gt__(self, other):
        return self.rule > other.rule

    def __ge__(self, other):
        return self.rule >= other.rule

    # prevod vsetkych komponent na male pismena
    def lower(self):
        self.source = self.source.lower()
        self.symbol = self.symbol.lower()
        self.dest = self.dest.lower()
        self.rule = self.rule.lower()
        return self


################################################################################
################################################################################

# spracovani argumentu
try:
    opts, args = getopt.getopt(sys.argv[1:], "edi", [
    "help", "input=", "output=", "no-epsilon-rules", "determinization", \
        "case-insensitive"])
except getopt.GetoptError:
    sys.exit(1)

eps_rules = False
determinize = False
ignore_case = False
input_file = sys.stdin
output_file = sys.stdout

for o, a in opts:
    if o == "--help":
        if len(opts) != 1:
            sys.exit(exit_codes["PARAMS_ERROR"])
        else:
            print_help()
    elif o == "--input":
        if input_file != sys.stdin:
            sys.exit(exit_codes["PARAMS_ERROR"])
        else:
            input_file = a
    elif o == "--output":
        if output_file != sys.stdout:
            sys.exit(exit_codes["PARAMS_ERROR"])
        output_file = a
    elif o in ("-e", "--no-epsilon-rules"):
        if eps_rules:
            sys.exit(exit_codes["PARAMS_ERROR"])
        else:
            eps_rules = True
    elif o in ("--determinization", "-d"):
        if determinize:
            sys.exit(exit_codes["PARAMS_ERROR"])
        else:
            determinize = True
    elif o in ("-i", "--case-insensitive"):
        if ignore_case:
            sys.exit(exit_codes["PARAMS_ERROR"])
        else:
            ignore_case = True
    else:
        sys.exit(exit_codes["PARAMS_ERROR"])

if len(args) != 0:
    sys.exit(exit_codes["PARAMS_ERROR"])

# kontrola vzajemne se vylucujicich parametrov -e a -d
if eps_rules and determinize:
    sys.exit(exit_codes["PARAMS_ERROR"])

# otevreni souboru pro cteni a pro zapis
if input_file != sys.stdin:
    try:
        infile = open(input_file, "r")
    except:
        sys.exit(exit_codes["INPUT_ERROR"])
else:
    infile = sys.stdin

if output_file != sys.stdout:
    try:
        outfile = open(output_file, "w")
    except:
        sys.exit(exit_codes["OUTPUT_ERROR"])
else:
    outfile = sys.stdout

# retezec obsahujuci automat
fsm_str = infile.read()
# index v retezci fsm
fsm_idx = -1
# stavy automatu
state = 'INIT'

# skompilovane regularene vyrazy
re_state = re.compile("^(\w+)")
re_input = re.compile("(^[^(){}>,.'|# \-\n\t])|(?:\'([^'])\')|(?:\'(\'\')\')")
re_rules = re.compile("^(\w+)(?:\s+([^[^(){}>,.'|# \-\n\t])|\
\s*\'\'|\s*\'([^'])\'|\s*\'(\'\')\')?\s*->\s*(\w+)")

# indikator nacitania ciarky
comma_read = False

# mnozina stavov automatu
fsm_states = set()
fsm_input = set()
fsm_rules = set()
fsm_start_state = None
fsm_fstates = set()

konec = False
while 1:
    #if not konec:
    if fsm_idx != len(fsm_str)-1:
        fsm_idx += 1
        char = fsm_str[fsm_idx]
    else:
        konec = True
    if state == 'INIT':
        # test na komentar a bily znak
        if char == '#':
            state = 'COMM'
            nstate = 'INIT'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'INIT'
        elif char == '(':
            state = 'FSM_START'
        else:
            state = 'FAIL' # syn
        pstate = 'INIT'


    ########################################################
    # zacatek nacitani konecneho automatu
    elif state == 'FSM_START':
        if char == '#':
            state = 'COMM'
            nstate = 'FSM_START'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'FSM_START'
        # nacteni mnoziny stavu
        elif char == '{':
            state = 'READ_STATES_SET'
        else:
            state = 'FAIL' # syn
        pstate = 'FSM_START'


    ########################################################
    # pokracovanie na dalsiu mnozinu
    elif state == 'FSM_CONT':
        if char == '#':
            state = 'COMM'
            nstate = 'FSM_CONT'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'FSM_CONT'
        elif char == ',':
            if comma_read == True:
                state = 'FAIL' # syn
            else:
                comma_read = True
                state = 'FSM_CONT'
        elif char == '{':
            if comma_read == False:
                state = 'FAIL' # syn
            else:
                comma_read = False
                state = pstate
                pstate = 'FSM_CONT'
        else:
            state = 'FAIL' # syn


    ########################################################
    # nacitanie a ulozenie mnoziny stavov
    elif state == 'READ_STATES_SET':
        if char == '#':
            state = 'COMM'
            nstate = 'READ_STATES_SET'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'READ_STATES_SET'
        elif char == ',':
            if comma_read == True or pstate == 'FSM_START':
                state = 'FAIL' # syn
            else:
                comma_read = True
                state = 'READ_STATES_SET'
        elif char == '}':
            if comma_read == True:
                state = 'FAIL' # syn
            else:
                state = 'FSM_CONT'
                pstate = 'READ_INPUT_SET'
        else:
            if comma_read == False and pstate != 'FSM_START':
                state = 'FAIL' # syn
            else:
                mo = re_state.match(fsm_str[fsm_idx:])
                if mo == None or fsm_str[fsm_idx+mo.end()-1] == '_'\
                   or fsm_str[fsm_idx] == '_':
                    state = 'FAIL' # syn
                else:
                    fsm_states.add(mo.group(1))
                    fsm_idx += mo.end()-1
                    comma_read = False
                    state = 'READ_STATES_SET'
                    pstate = 'READ_STATES_SET'



    ########################################################
    # nacitanie a ulozenie vstupnej abecedy
    elif state == 'READ_INPUT_SET':
        if char == '#':
            state = 'COMM'
            nstate = 'READ_INPUT_SET'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'READ_INPUT_SET'
        elif char == ',':
            if comma_read == True or pstate == 'FSM_CONT':
                state = 'FAIL' # syn
            else:
                comma_read = True
                state = 'READ_INPUT_SET'
        elif char == '}':
            if comma_read == True:
                state = 'FAIL' # syn
            else:
                state = 'FSM_CONT'
                pstate = 'READ_FSM_RULES'
        else:
            if comma_read == False and pstate != 'FSM_CONT':
                state = 'FAIL' # syn
            else:
                mo = re_input.match(fsm_str[fsm_idx:])
                if mo == None:
                    state = 'FAIL' # syn
                else:
                    if mo.group(1) != None:
                        fsm_input.add(mo.group(1))
                    elif mo.group(2) != None:
                        fsm_input.add(mo.group(2))
                    elif mo.group(3) != None:
                        fsm_input.add(mo.group(3))
                    fsm_idx += mo.end()-1
                    comma_read = False
                    state = 'READ_INPUT_SET'
                    pstate = 'READ_INPUT_SET'


    ########################################################
    # nacitanie mnoziny pravidiel
    elif state == 'READ_FSM_RULES':
        if char == '#':
            state = 'COMM'
            nstate = 'READ_FSM_RULES'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'READ_FSM_RULES'
        elif char == ',':
            if comma_read == True or pstate == 'FSM_CONT':
                state = 'FAIL' # syn
            else:
                comma_read = True
                state = 'READ_FSM_RULES'
        elif char == '}':
            if comma_read == True:
                state = 'FAIL' # syn
            else:
                state = 'READ_START_STATE'
                pstate = 'READ_START_STATE'
        else:
            if comma_read == False and pstate != 'FSM_CONT':
                state = 'FAIL' # syn
            else:
                mo = re_rules.match(fsm_str[fsm_idx:])
                if mo == None:
                    state = 'FAIL' # syn
                else:
                    source = mo.group(1)
                    symbol = mo.group(2) or mo.group(3) or mo.group(4)
                    dest = mo.group(5)
                        
                    fsm_rules.add(Rules(source, symbol, dest))
                    fsm_idx += mo.end()-1
                    comma_read = False
                    state = 'READ_FSM_RULES'
                    pstate = 'READ_FSM_RULES'


    ########################################################
    # nacteni startovaciho stavu
    elif state == 'READ_START_STATE':
        if char == '#':
            state = 'COMM'
            nstate = 'READ_START_STATE'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'READ_START_STATE'
        elif char == ',':
            if comma_read == True:
                state = 'FAIL' # syn
            else:
                comma_read = True
                state = 'READ_START_STATE'
        else:
            if comma_read == False:
                state = 'FAIL' # syn
            else:
                mo = re_state.match(fsm_str[fsm_idx:])
                if mo == None: 
                    state = 'FAIL' # syn
                else:
                    fsm_start_state = mo.group(1)
                    fsm_idx +=mo.end()-1
                    comma_read = False
                    state = 'FSM_CONT'
                    pstate = 'READ_END_SET'


    ########################################################
    # nacitanie koncovych stavov
    elif state == 'READ_END_SET':
        if char == '#':
            state = 'COMM'
            nstate = 'READ_END_SET'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'READ_END_SET'
        elif char == ',':
            if comma_read == True or pstate == 'FSM_CONT':
                state = 'FAIL' # syn chyba
            else:
                comma_read = True
                state = 'READ_END_SET'
        elif char == '}':
            if comma_read == True:
                state = 'FAIL' # syn chyba
            else:
                brace_read = False
                state = 'FSM_END'
        else:
            if comma_read == False and pstate != 'FSM_CONT':
                state = 'FAIL' # syn chyba
            else:
                mo = re_state.match(fsm_str[fsm_idx:])
                if mo == None or fsm_str[fsm_idx+mo.end()-1] == '_'\
                   or fsm_str[fsm_idx] == '_':
                    state = 'FAIL' # syntakticka chyba
                else:
                    fsm_fstates.add(mo.group(1))
                    fsm_idx += mo.end()-1
                    comma_read = False
                    state = 'READ_END_SET'
                    pstate = 'READ_END_SET'

    ########################################################
    # koniec popisu automatu
    elif state == 'FSM_END':
        if konec:
            state = 'FSM_OK'
            continue
        if char == '#':
            state = 'COMM'
            nstate = 'FSM_END'
        elif char.isspace():
            state = 'WHITE'
            nstate = 'FSM_END'
        elif char == ')':
            if brace_read:
                state = 'FAIL'
            else:
                state = 'FSM_END'

        elif char == '':
            state = 'FSM_OK'
        else:
            state = 'FAIL'

    ########################################################
    # odstranenie komentarov
    elif state == 'COMM':
        while fsm_str[fsm_idx] != "\n" and fsm_idx != len(fsm_str)-1:
            fsm_idx += 1
        if fsm_idx == len(fsm_str)-1:
            konec = True
        state = nstate

    # preskocenie bielych znakov
    elif state == 'WHITE':
        while fsm_str[fsm_idx].isspace() and fsm_idx != len(fsm_str)-1:
            fsm_idx += 1
        if fsm_idx == len(fsm_str)-1:
            konec = True
        fsm_idx -= 1
        state = nstate

    # automat nesplnuje vstupni pozadavky -> ukonceni skriptu
    elif state == 'FAIL':
        #print('FAIL')
        print("Chyba: nespravne zadany vstupny automat", file=sys.stderr)
        sys.exit(exit_codes["SYN_ERROR"])

    elif state == 'FSM_OK':
        break

################################################################################
# prevedenie na male pismena, ak bol zadany -i
if ignore_case:
    # prevod stavov
    for old in fsm_states.copy():
        fsm_states.remove(old)
        fsm_states.add(old.lower())

    # vstupna abeceda
    for old in fsm_input.copy():
        fsm_input.remove(old)
        fsm_input.add(old.lower())

    # pravidla
    for old in fsm_rules.copy():
        fsm_rules.remove(old)
        fsm_rules.add(old.lower())

    # pocatecni stav
    fsm_start_state.lower()

    # mnozina koncovych stavu
    for old in fsm_fstates.copy():
        fsm_fstates.remove(old)
        fsm_fstates.add(old.lower())


################################################################################
# kontrola semantickej spravnosti vstupu

# kontrola neprazdnosti vstupnej abecedy
if len(fsm_input) == 0:
    print("Chyba: vstupna abeceda je prazdna.", file=sys.stderr)
    sys.exit(exit_codes["SEM_ERROR"])

# kontrola disjunktnosti prvni a druhe komponenty
if len(fsm_states.intersection(fsm_input)) != 0:
    print("Chyba: mnozina stavov a vstupna abeceda nie su disjunktne.",\
          file=sys.stderr)
    sys.exit(exit_codes["SEM_ERROR"])

# je startujici stav v mnozine stavov ?
if fsm_start_state not in fsm_states:
    print("Chyba: startujuci stava sa nenachzdza v nozine stavov.",\
          file=sys.stderr)
    sys.exit(exit_codes["SEM_ERROR"])

# je mnozina koncovych stavov podmnozinou vsetkych stavov ?
if not fsm_fstates.issubset(fsm_states):
    print("Chyba: množina koncových stavov nie je podmnožinou všetkých stavov.",\
          file=sys.stderr)
    sys.exit(exit_codes["SEM_ERROR"])
    
# kontrola pravidel automatu
for r in fsm_rules:
    if r.source not in fsm_states or \
       r.symbol not in fsm_input and r.symbol != '' or \
       r.dest not in fsm_states:
        print("Chyba: chybne zadané pravidla konečného automatu.", \
              file=sys.stderr)

################################################################################
# odstranenie epsilon prechodov
################################################################################
if determinize or eps_rules:
    Fnew = set()
    R = set()
    for p in fsm_states:
        # vytvorime epsilon uzaver daneho stavu
        Qold = set()
        Qold.add(p)
        Q = []
        Q.append(Qold)
        while True:
            Qnew = set() # nova mnozina stavov
            for qe in Q[-1]: # iterovanie cez staru mnozinu
                for r in fsm_rules: # iterovanie cez pravidla
                    if r.source == qe and r.symbol == '':
                        Qnew.add(r.dest)
            Qnew = Qnew.union(Q[-1])
            if Qnew == Q[-1]:
                p_closure = Qnew
                break
            else:
                Q.append(Qnew)

        # odstraneni epsilon prechodov
        Rnew = set()
        for q_clos in p_closure:
            for r in fsm_rules:
                if q_clos == r.source and r.symbol != '':
                    Rnew.add(Rules(p, r.symbol, r.dest))
        R = R.union(Rnew)
        if len(p_closure.intersection(fsm_fstates)) != 0:
            Fnew.add(p)

    # nahradenie povodnych mnozin novymi
    fsm_fstates = Fnew
    fsm_rules = R

################################################################################
################################################################################

################################################################################
# determinizace automatu
################################################################################
if determinize:
    Qnew = set()
    Qnew.add(fsm_start_state)
    Rd = set()
    Qd = set()
    Fd = set()
    while True:
        Qc = set()
        temp = Qnew.pop() # Qc = Q'
        if type(temp) == str:
            Qc.add(temp)
        else:
            Qc = temp
        return_gen_state = gen_state(Qc)
        Qd.add(gen_state(Qc)) # pridani stavu medzi vysledne stavy
        for a in fsm_input:
            Qcc = set() # Qcc = Q''
            for p in Qc:
                for r in fsm_rules:
                    if p == r.source and a == r.symbol:
                        Qcc.add(r.dest)

            # pridanie noveho pravidla
            if len(Qcc) != 0:
                Rd.add(Rules(Qc, a, Qcc))

            # pridanie nonovytvoreneho stavu k cakatelom na Qd
            if gen_state(Qcc) not in Qd.union(set()):
                Qnew.add(frozenset(Qcc))

        # pridanie koncoveho stavu
        if len(Qc.intersection(fsm_fstates)) != 0:
            Fd.add(gen_state(Qc))

        if len(Qnew) == 0:
            break

    # odstranenie prazdneho stavu
    try:
        Qd.remove('')
    except:
        pass
    # nahradenie povodnych hodnot novymi
    fsm_states = Qd
    fsm_fstates = Fd
    fsm_rules = Rd


################################################################################
################################################################################

print_fsm(fsm_states, fsm_input, fsm_rules, fsm_start_state, fsm_fstates)

sys.exit(exit_codes["ALL_OK"])
