% Autor: Peter Lacko
% Rok: 2011/2012

%delka linearniho seznamu
delka([],0).
delka([_|T],S) :- delka(T,SS), S is SS + 1.

%je prvek clenem lin. seznamu?
jePrvek([X|_],X).
jePrvek([_|T],X) :- jePrvek(T,X). 

%spojeni dvou linearnich seznamu
spoj([],L,L).
spoj([H|T],L,[H|TT]) :- spoj(T,L,TT).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Vytvorte funkci pro rozdeleni linearniho seznamu na poloviny
% divide_half(INPUT,HALF1,HALF2)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
divide_half2([],[],[],[]).                                        % koniec delenia 
divide_half2([],L,[C|L2],[C|T2]) :- divide_half2([],L,L2,T2).     % pokracujeme vo vytvarani druhej polovice
divide_half1([],L,L2,T2) :- divide_half2([],L,L2,T2).             % skopirovanie posledneho prvku ak je sudy pocet
divide_half1([_,_|T],[C|L],L2,[C|T2]) :- divide_half1(T,L,L2,T2). % pokracujeme vo vytvarani prvej polovice
divide_half1([_],[C|L],L2,[C|T2]) :- divide_half2([],L,L2,T2).    % skopirovanie posledneho prvku, ak je lichy pocet
divide_half(I,H1,H2) :- divide_half1(I,H1,H2,I).                  % vytvorenie pomocneho zoznamu


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Vytvorte funkci pro odstraneni obecneho prvku z obecneho seznamu
% remove_item_GEN(INPUT,ITEM,OUTPUT)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
remove_item_GEN([],_,[]). % koniec zoznamu
remove_item_GEN([X|T],X,L) :- remove_item_GEN(T,X,L),!. % odrezanie hlavy
% ak je hlava zoznam, kontrola hlavy
remove_item_GEN([H|T],X,[L|L2]) :- is_list(H), remove_item_GEN(H,X,L), remove_item_GEN(T,X,L2).
% kontrola zbytku zoznamu
remove_item_GEN([H|T],X,[H|L]) :- remove_item_GEN(T,X,L).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Vytvorte funkci pro reverzaci obecneho seznamu
% reverse_GEN(INPUT,OUTPUT)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
reverse_GEN([],[]).
reverse_GEN([H|T],X) :- is_list(H), reverse_GEN(T,L1), reverse_GEN(H,H2), !, spoj(L1,[H2],X).  
reverse_GEN([H|T],X) :- reverse_GEN(T,L1), spoj(L1,[H],X), !.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Vytvorte funkci pro vlozeni prvku na n-tou pozici linearniho seznamu
% insert_pos(LIST,POSITION,ITEM,OUTPUT)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
insert_pos(T,1,I,[I|T]). % vlozeni prvku
% rekurzivne zanorovanie do POSITION urovne­
insert_pos([H|T],P,I,[H|T2]) :- R is P-1, insert_pos(T,R,I,T2).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Vytvorte funkci pro inkrementaci kazdeho prvku obecneho seznamu o hodnotu hloubky zanoreni prvku
% increment_general(INPUT,OUTPUT)
% input [0,0,[0]] -> output [1,1,[2]]
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
increment_general([],[],_).
% ak je hlava zoznam, zanor sa do hlavy a inkrementuj hlbku zanorenia
increment_general([H|T],[L|L2],VAL) :- is_list(H), VAL2 is VAL+1, increment_general(H,L,VAL2), increment_general(T,L2,VAL),!.
% inkrementovanie hodnot zoznamu
increment_general([H|T],[C|L],VAL) :- increment_general(T,L,VAL), C is H + VAL.
increment_general(I,O) :- increment_general(I,O,1). % inicializacia hlbky na 1
