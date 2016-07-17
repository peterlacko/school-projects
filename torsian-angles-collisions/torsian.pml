# Uloha 2 do predmetu PBI
#
# Autor: Peter Lacko
# 17.11.2015
#
# Skript na zobrazenie tripeptidu WLW a detekciu kolizii atomov pri zmene torznych uhlov Fi a Psi.
# Pri detekcii kolizii medzi tryptofanmi a leucinom nebola brana do uvahy hlavny retazec proteinu,
# kedze "susediace" atomy su vzdy v kolizii.
# Vystupom ulohy je tento skript a grafy so znazornenymi koliziami medzi dvoma tryptofanmi a prvym
# tryptofanom a leucinom (zmena uhla Fi).
# Graf s koliziami druheho tryptofanu a leucinu (zmena Psi) nie je prilozeny nakolko v tomto pripade
# nedoslo k ziadnej kolizii.
# pocet kolizii: TRP x TRP: 567
#                TRP1x LEU:   5
#                TRP3x LEU:   0


fab WLW, wlw
color white
color red, name c
color green, name n
color yellow, name ca
color blue, name h
show sticks
hide sticks, sidechain
show sphere
set sphere_scale, 0.25, (all)
hide sphere, sidechain

# Vyberiem hlavny retazec
select /wlw///LEU`2/C or /wlw///LEU`2/CA or /wlw///LEU`2/N or /wlw///LEU`2/H or /wlw///LEU`2/O or /wlw///TRP`1/CA or /wlw///TRP`1/C or /wlw///TRP`1/O or /wlw///TRP`3/CA or /wlw///TRP`3/N or /wlw///TRP`3/H

g_main_chain = cmd.index("sele")
g_main_chain = [i[1] for i in g_main_chain]

# Ulozim vdw vzdialenosti do premennej stored.vdw
stored.vdw = {}
iterate wlw, stored.vdw[index] = vdw

#  Funkcia kontroluje ci aspon jeden z danych atomov lezi na hlavnom retazci
def on_main(atom1, atom2, main_chain):\
    if atom1[1] in main_chain:\
        return True\
    if atom2[1] in main_chain:\
        return True\
    return False

# Funkcia kontroluje ci obe atomy lezia na hlavnom retazci
def both_on_main(atom1, atom2, main_chain):\
    s = set([atom1[1], atom2[1]])\
    if s.issubset(set(main_chain)):\
        return True\
    return False


# Detekujeme kolizie atomov medzi dvoma aminokyselinami.
# Ak je nastaveny parameter exclude_main = True, neberieme do uvahy kolizie atomov na hlavnom retazci.
def is_colliding(acid1, acid2, exclude_main = False):\
    for atom1 in cmd.index(acid1):\
        for atom2 in cmd.index(acid2):\
            dist = cmd.get_distance("%s`%d"%atom1, "%s`%d"%atom2)\
            if dist < (stored.vdw[atom1[1]] + stored.vdw[atom2[1]]):\
                if exclude_main:\
                    if on_main(atom1, atom2, g_main_chain):\
                        continue\
                return True\
    return False
            

# Detekujeme kolizie medzi tryptofanmi. Do uvahy berieme aj kolizie na hlavnom retazci, nakolko su pomerne
# caste kolizie "koncovych" atomov O a H.
trp1_trp3 = ""
for angle1 in range(0, 360, 10):\
    cmd.set_dihedral("/wlw///TRP`1/C", "/wlw///LEU`2/N", "/wlw///LEU`2/CA", "/wlw///LEU`2/C", angle1)\
    for angle2 in range(0, 360, 10):\
        cmd.set_dihedral("/wlw///LEU`2/N", "/wlw///LEU`2/CA", "/wlw///LEU`2/C", "/wlw///TRP`3/N", angle2)\
        if (is_colliding("/wlw///TRP`1", "/wlw///TRP`3")):\
            trp1_trp3 += (str(angle1) + ", " + str(angle2) + "\n")

# Detekujeme kolizie medzi prvym tryptofanom a leucinom, neberieme do uvahy kolizie na kostre.
trp1_leu = ""
for angle1 in range(0, 360, 10):\
    cmd.set_dihedral("/wlw///TRP`1/C", "/wlw///LEU`2/N", "/wlw///LEU`2/CA", "/wlw///LEU`2/C", angle1)\
    if (is_colliding("/wlw///TRP`1", "/wlw///LEU`2", exclude_main = True)):\
        trp1_leu += (str(angle1) + "\n")

# Detekujeme kolizie medzi druhym tryptofanom a leucinom, neberieme do uvahy kolizie na kostre.
trp3_leu = ""
for angle2 in range(0, 360, 10):\
    cmd.set_dihedral("/wlw///LEU`2/N", "/wlw///LEU`2/CA", "/wlw///LEU`2/C", "/wlw///TRP`3/N", angle2)\
    if (is_colliding("/wlw///TRP`3", "/wlw///LEU`2", exclude_main = True)):\
        trp3_leu += (str(angle2) + "\n")

# Zapiseme vysledky zisteni do csv suborov pre pohodlne vykreslenie.
python
with open("trp1_trp3.csv", "w") as f:
    f.write(trp1_trp3)
with open("trp1_leu.csv", "w") as f:
    f.write(trp1_leu)
with open("trp3_leu.csv", "w") as f:
    f.write(trp3_leu)
python end
