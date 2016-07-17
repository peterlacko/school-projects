-- Skript pro vytvoreni MySQL databaze
-- Petr Holubec (xholub26), Peter Lacko (xlacko06)

-- Odstraneni starych objektu
DROP TABLE IF EXISTS oblibene;
DROP TABLE IF EXISTS user;
DROP TABLE IF EXISTS clanky;
DROP TABLE IF EXISTS citaty;
DROP TABLE IF EXISTS akce;
DROP TABLE IF EXISTS dod;
DROP TABLE IF EXISTS predmet;
DROP TABLE IF EXISTS obor;
DROP TABLE IF EXISTS fakulta;
DROP TABLE IF EXISTS skola;
DROP TABLE IF EXISTS zamereni;
DROP PROCEDURE IF EXISTS PROC_ZMEN_JMENO;
DROP TRIGGER IF EXISTS TRIG_ZMEN_JMENO;

-- Vytvoreni tabulek
CREATE TABLE user ( 
    user_id         INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    username        VARCHAR(30) NOT NULL,
    password        VARCHAR(50) NOT NULL,
    fullname        VARCHAR(60) NOT NULL,
    role	    	VARCHAR(20) NOT NULL
);
    
CREATE TABLE zamereni (
    Nazev_zamereni  VARCHAR(30) NOT NULL PRIMARY KEY,
    Popis_zamereni  VARCHAR(80),
    Uplatneni       VARCHAR(50)
);

CREATE TABLE skola (
    Nazev_skoly     VARCHAR(50) NOT NULL PRIMARY KEY,
    Adresa_skoly    VARCHAR(50) NOT NULL,
    Email_skoly     VARCHAR(40),
    Web_skoly       VARCHAR(20),
    Rektor          VARCHAR(40) NOT NULL,
    Nazev_zamereni  VARCHAR(30),
    
    FOREIGN KEY (Nazev_zamereni) REFERENCES
    zamereni (Nazev_zamereni)
);

CREATE TABLE fakulta (
    ID_fakulty      INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Nazev_fakulty   VARCHAR(60) NOT NULL,
    Adresa_fakulty  VARCHAR(50),
    Email_fakulty   VARCHAR(40),
    Web_fakulty     VARCHAR(20),
    Dekan           VARCHAR(40) NOT NULL,
    Nazev_skoly     VARCHAR(50) NOT NULL,
    
    FOREIGN KEY (Nazev_skoly) REFERENCES
    skola (Nazev_skoly) ON DELETE CASCADE
);

CREATE TABLE obor (
    ID_oboru        INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Nazev_oboru     VARCHAR(40) NOT NULL,
    Delka_studia    INTEGER NOT NULL,
    Forma_studia    VARCHAR(15),
    Titul           VARCHAR(10),
    Zakonceni       VARCHAR(20),
    Popis           VARCHAR(300),
    ID_fakulty      INTEGER NOT NULL,
    
    FOREIGN KEY (ID_fakulty) REFERENCES
    fakulta (ID_fakulty) ON DELETE CASCADE
);

CREATE TABLE predmet (
    ID_predmetu         INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Zkratka_predmetu    VARCHAR(5),
    Nazev_predmetu      VARCHAR(30) NOT NULL,
    Kredity             INTEGER NOT NULL,
    Ukonceni            VARCHAR(10),
    Garant              VARCHAR(30) NOT NULL,
    ID_fakulty          INTEGER NOT NULL,
    
    FOREIGN KEY (ID_fakulty) REFERENCES
    fakulta (ID_fakulty) ON DELETE CASCADE
);

CREATE TABLE akce (
    ID_akce        INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Titulek         VARCHAR(30) NOT NULL,
    Text            VARCHAR(200),
    Kdy				DATETIME NOT NULL,
    ID_fakulty      INTEGER NOT NULL,
    
    FOREIGN KEY (ID_fakulty) REFERENCES
    fakulta (ID_fakulty) ON DELETE CASCADE
);

CREATE TABLE dod (
    ID_DOD      INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Zacatek     DATETIME NOT NULL,
    Konec       DATETIME,
    Podrobnosti VARCHAR(200),
    ID_fakulty  INTEGER NOT NULL,
    
    FOREIGN KEY (ID_fakulty) REFERENCES
    fakulta (ID_fakulty) ON DELETE CASCADE
);

CREATE TABLE citaty (
    ID_citatu   INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Citat 		VARCHAR(200) NOT NULL,
    Autor 		VARCHAR(40)
);

CREATE TABLE clanky (
    ID_clanku   INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Nazev		VARCHAR(100) NOT NULL,
    Uvod 		VARCHAR(400),
    Text 		VARCHAR(5000) NOT NULL,
    Autor 		VARCHAR(40) NOT NULL,
    Datum       DATE
);

CREATE TABLE oblibene (
	ID_oblibene INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY,
    User_id		INTEGER NOT NULL,
    Nazev_skoly VARCHAR(50),
    ID_fakulty  INTEGER,

    FOREIGN KEY (User_id) REFERENCES
    user (user_id) ON DELETE CASCADE,

	FOREIGN KEY (Nazev_skoly) REFERENCES
    skola (Nazev_skoly) ON DELETE CASCADE,

    FOREIGN KEY (ID_fakulty) REFERENCES
    fakulta (ID_fakulty) ON DELETE CASCADE
);

-- Naplneni obsahem
INSERT INTO user
VALUES(null,'admin','21232f297a57a5a743894a0e4a801fc3','admin','admin');
INSERT INTO user
VALUES(null,'user','5cc32e366c87c4cb49e4309b75f57d64','user','user');
INSERT INTO user
VALUES(null,'lojza','9178740077a7834cc1ab45db456ed56e','Lojza Testovací','user');

INSERT INTO zamereni
VALUES('Technické','Nejlepší je prostě technická vysoká škola.','Technické obory a disciplíny');
INSERT INTO zamereni
VALUES('Veterinární','Škola se zaměřením veterinářství a farmaceutiku.','Všude');
INSERT INTO zamereni
VALUES('Obecné','Mnoho kvalitních oborů s kvalitními pedagogy.','Skoro nikde, protože víte nic o všem');
INSERT INTO zamereni
VALUES('Zemědělské','Zemědělské a lesnické učení zaměřené na práci v přírodě.','Lesní správa, zemědělství');

INSERT INTO skola
VALUES('VUT Brno','Antonínská 548/1, Brno 601 90','vut@vutbr.cz','www.vutbr.cz','Karel Rais','Technické');
INSERT INTO skola
VALUES('Masarykova univerzita Brno','Žerotínovo nám. 617/9, 601 77 Brno','info@muni.cz','www.muni.cz','Mikuláš Bek','Obecné');
INSERT INTO skola
VALUES('Mendelova univerzita v Brně','Zemědělská 1/1665, 613 00 Brno','info@mendelu.cz','www.mendelu.cz','prof. Ing. Jaroslav Hlušek, CSc','Zemědělské');
INSERT INTO skola
VALUES('Technická univerzita v Liberci','Studentská 1402/2, 461 17 Liberec 1','info@tul.cz','www.tul.cz','Prof. Dr. Ing. Zdeněk Kůs','Technické');

INSERT INTO fakulta
VALUES(null,'Fakulta informačních technologií','Božetěchova 1/2, 612 66 Brno','info@fit.vutbr.cz','www.fit.vutbr.cz','Jan Zendulka','VUT Brno');
INSERT INTO fakulta
VALUES(null,'Fakulta podnikatelská','Kolejní 8, 612 00 Brno','info@fp.vutbr.cz','www.fp.vutbr.cz','FP Dekan','VUT Brno');
INSERT INTO fakulta
VALUES(null,'Fakulta stavební','Vevěří 8, 612 OO Brno','info@fast.vutbr.cz','www.fast.vutbr.cz','Rostislav Drochytka','VUT Brno');
INSERT INTO fakulta
VALUES(null,'Fakulta chemická','Purkyňova 464/118, 612 00 Brno','fch@fch.vutbr.cz','www.fch.vutbr.cz','prof. Ing. Jaromír Havlica, DrSc.','VUT Brno');
INSERT INTO fakulta
VALUES(null,'Fakulta architektury','Poříčí 273/5, 639 00 Brno','fa@fa.vutbr.cz','www.fa.vutbr.cz','doc. Ing. Josef Chybík, CSc.','VUT Brno');
INSERT INTO fakulta
VALUES(null,'Fakulta právní','Brněnská 4, 612 00 Brno','fp@fp.muni.cz','www.fp.muni.cz','FP Dekan','Masarykova univerzita Brno');
INSERT INTO fakulta
VALUES(null,'Přírodovědecká fakulta','Brněnská 4, 612 00 Brno','pf@pf.muni.cz','www.pf.muni.cz','PF Dekan','Masarykova univerzita Brno');
INSERT INTO fakulta
VALUES(null,'Lékařská fakulta','Brněnská 5, 612 00 Brno','lf@lf.muni.cz','www.lf.muni.cz','LF Dekan','Masarykova univerzita Brno');
INSERT INTO fakulta
VALUES(null,'Fakulta strojní','Studentská 1402/2, 461 17 Liberec I','fakulta@fs.tul.cz','www.fs.tul.cz','doc. Ing. Miroslav MALÝ, CSc.','Technická univerzita v Liberci');
INSERT INTO fakulta
VALUES(null,'Fakulta mechatroniky, informatiky a mezioborových studií','Studentská 2, 461 17 Liberec','dotazy@fm.tul.cz','www.fm.tul.cz','prof. Ing. Václav Kopecký, CSc.','Technická univerzita v Liberci');

INSERT INTO obor
VALUES(null,'Informační technologie',3,'prezenční','Bc.','Státnice','Bakalářský studijní program Informační technologie je zaměřen na výchovu absolventů, kteří se mohou v praxi uplatnit jako projektanti, konstruktéři, programátoři a údržbáři počítačových systémů.',1);
INSERT INTO obor
VALUES(null,'Klikání tlačítkem myši',4,'prezenční','Ing.','Státnice','Studijní obor Klikání tlačítkem myši je oborem navazujícího magisterského studijního programu Informační technologie.',1);
INSERT INTO obor
VALUES(null,'Podnikání',3,'dálkové','Bc.','Bakalářská práce','Cílem bakalářského studijního oboru Podnikání je poskytnout uchazečům s ukončeným středoškolským vzděláním možnost bakalářského studia, v jehož průběhu student získá teoretické i praktické znalosti z oblasti ekonomiky podniku.',2);
INSERT INTO obor
VALUES(null,'Tunelování',3,'dálkové','Bc.','Bakalářská práce','Cílem bakalářského studijního oboru Tunelování je poskytnout uchazečům s ukončeným středoškolským vzděláním vysokoškolské studium, v jehož průběhu student získá praktické a vzájemně provázané odborné znalosti.',3);
INSERT INTO obor
VALUES(null,'Šamanství',1,'prezenční','PhDr','Státnice','Šamanismus je souhrn náboženských, magických, mystických a mytologických představ a rituálních praktik, známý u mnoha kultur v různých oblastech světa a existující nejspíš už od dob paleolitických lovců a sběračů.',8);
INSERT INTO obor
VALUES(null,'Čínské lidové léčitelství',9,'prezenční','PhDr','Státnice','Rady a recepty tradiční čínské lidové medicíny přinášejí výjimečnou sbírku lidových receptů z nejproslulejší léčitelské tradice světa.',8);
INSERT INTO obor
VALUES(null,'Ufologie',3,'dálkové','Bc.','Bakalářská práce','Ufologove by meli spolupracovat se spickovymi odborniky z ruznych oboru a s jejich pomoci se snazit jevy konkretizovat.  Je vcelku jedno jestli se ufolog soustreduje na UFO, paranormalni jevy nebo unosy lidi do UFO.',7);

INSERT INTO predmet
VALUES(null,'IDS','Databázové systémy',5,'ZaZk','Jan Zendulka',1);
INSERT INTO predmet
VALUES(null,'ITU','Tvorba uživatelského rozhraní',4,'KlZa','Víťa Beran',1);
INSERT INTO predmet
VALUES(null,'IMS','Modelování a simulace',5,'ZaZk','Petr Peringer',1);
INSERT INTO predmet
VALUES(null,'ISA','Síťové aplikace a správa sítí',5,'ZaZk','Petr Matoušek',1);
INSERT INTO predmet
VALUES(null,'EKON','Ekonomie',4,'Za','Honza Novak',2);
INSERT INTO predmet
VALUES(null,'IAL','Algoritmy',5,'ZaZk','Jan Maxmilián Honzík',1);
INSERT INTO predmet
VALUES(null,'IFJ','Formální jazyky a překladače',5,'ZaZk','Alexander Meduna',1);
INSERT INTO predmet
VALUES(null,'TUTU','Tunelování tunelů',3,'Zk','Jan Tunel',3);
INSERT INTO predmet
VALUES(null,'TUDA','Tunelování dálnic',3,'Zk','Michal Dálnice',3);
INSERT INTO predmet
VALUES(null,'MMN','Mimozemšťani mezi náma',2,'Zk','Ondřej Zmarsů',7);
INSERT INTO predmet
VALUES(null,'OVO','Odkazy v obilí',4,'Zá','Samuel Ječmen',7);
INSERT INTO predmet
VALUES(null,'ZANE','Základy neurologie',6,'Zá','Josef Kolařík',8);
INSERT INTO predmet
VALUES(null,'ZABI','Základy biochemie',7,'Zk','Adrian Wong',8);
INSERT INTO predmet
VALUES(null,'LIST','Stavba lidského těla',5,'Zk','Dezider Ivanovič Sedminohý',8);
INSERT INTO predmet
VALUES(null,'ALG','Algoritmy',5,'ZaZk','Josef Houba',10);
INSERT INTO predmet
VALUES(null,'GRA','Grafika a multimédia',6,'Zk','Tomáš Červený',10);
INSERT INTO predmet
VALUES(null,'CHKN','Chemie kolem nás',3,'Zá','Jan Dusík',4);
INSERT INTO predmet
VALUES(null,'VYB','Výbuchy a jiné exploze',4,'ZáZk','Petr Bum',4);
INSERT INTO predmet
VALUES(null,'JED','Teorie jedů',4,'Zk','Pavel Nemámtěrád',4);

INSERT INTO akce
VALUES(null,'Den zavřených dveří','Akce pořádaná studenty pro studenty','2012-5-10 12:00:00',1);
INSERT INTO akce
VALUES(null,'Běh na padesáttrojku','Závod s kuframa na autobus číslo třiapadesát','2012-5-2 14:30:00',2);
INSERT INTO akce
VALUES(null,'Veřejná pitva','Pravidelní akce pitvání mrtvoly je znova tady','2012-3-29 13:15:00',8);
INSERT INTO akce
VALUES(null,'Pozorování meteorického roje','Pozorování meteorického roje, aneb Superman se vrací','2012-1-13 23:48:25',7);
INSERT INTO akce
VALUES(null,'Konec světa','Nenechte si ujít dlouho očekávané představení, v hlavní roli: Vy!','2012-12-21 00:00:00',6);
INSERT INTO akce
VALUES(null,'Jak rychle zbohatnout','Úvod do světa velkého podnikání, aneb vstupte s námi do politiky!','2012-4-20 10:00:00',2);

INSERT INTO dod
VALUES(NULL,'2012-2-3 08:00:00','2012-2-3 18:00:00','Tradiční den otevřených dveří',1);
INSERT INTO dod
VALUES(NULL,'2012-3-25 09:00:00','2012-3-25 16:00:00','Den ukázky fakulty vnějšímu světu',2);
INSERT INTO dod
VALUES(null,'2012-1-15 10:00:00','2012-1-15 17:00:00','Stavebka se ukazuje světu',3);
INSERT INTO dod
VALUES(null,'2012-2-13 08:30:00','2012-2-13 16:00:00','DOD spolu s ukázkou výtvorů mechatroniků',10);
INSERT INTO dod
VALUES(null,'2012-1-11 12:00:00','2012-1-11 20:00:00','DOD na téma Máte právo nevypovídat',6);
INSERT INTO dod
VALUES(null,'2012-3-8 08:00:00','2012-3-8 15:00:00','Přijďte se podívat na pitvu na vlastní oči',8);

INSERT INTO citaty
VALUES(null,'Kde blb, tam nebezpečno.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Proti blbosti i bohové bojují marně.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Nad lidskou blbostí se taky nedá zvítězit. Ale nikdy se nesmí přestat proti ní bojovat.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Ono je lepší mluvit s chytrým člověkem o něčem hloupém než s hlupákem o něčem chytrém.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Smích chytré lidi léčí a jen blbce uráží.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Paměť je výsadou blbých. Chytrý nemá čas si pamatovat, chytrý musí vymýšlet.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Mládí biologické neosvobozuje od blbosti a hlouposti, stejně jako stáří nezaručuje moudrost. Jsou blbí dědkové a hloupé děti.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Nejhorší srážka v životě, je srážka s blbcem. Blbce nikdy neusvědčíte z blbství. Z takové srážky, vyjdete vždycky jako největší blbec pod sluncem.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Na světě je zavedeno, že spousta hloupých si hraje na chytré. Z chytrých, kterých je na světě nedostatek, si jen ti nejchytřejší hrají na hloupé.','Jan Werich');
INSERT INTO citaty
VALUES(null,'Rozdávat rady je zbytečné. Moudrý si poradí sám a hlupák stejně neposlechne.','Mark Twain');
INSERT INTO citaty
VALUES(null,'Hlupák má před vzdělaným člověkem velkou výhodu - je sám se sebou vždy spokojený.','Napoleon Bonaparte');
INSERT INTO citaty
VALUES(null,'Každý člověk dělá chyby, ale jen hlupák u nich zůstává.','Marcus Tullius Cicero');
INSERT INTO citaty
VALUES(null,'Nic není nesnesitelnějšího než hlupák, který má štěstí.','Seneca');
INSERT INTO citaty
VALUES(null,'Lépe mlčet a být považován za hlupáka, než promluvit a odstranit všechny pochybnosti.','Abraham Lincoln');
INSERT INTO citaty
VALUES(null,'V politice není hloupost handicap.','Napoleon Bonaparte');

INSERT INTO oblibene
VALUES(NULL,2,NULL,1);
INSERT INTO oblibene
VALUES(NULL,3,NULL,2);
INSERT INTO oblibene
VALUES(NULL,2,NULL,10);
INSERT INTO oblibene
VALUES(NULL,2,'Masarykova univerzita Brno',NULL);

INSERT INTO clanky
VALUES(null,'Jak vybrat obor','Stejné obory se dají studovat na různých fakultách, proto se <b>nejprve rozhodněte o oboru, a potom se zaměřte na výběr konkrétní fakulty</b>.
Pokud si nejste jisti nebo vůbec nemáte představu o tom, co byste chtěli studovat, zkuste následující postup.','<h3>Pojmenujte a popište své schopnosti a zájmy</h3>
<p>Pojmenujte a vypište si své schopnosti a zájmy, odpovězte si na otázky: jaké předměty se vám ve škole líbí, co vás baví dělat. Zkuste si představit svou budoucí profesi.</p>

<h3>Vyberte si zaměření oborů</h3>
<p>Určete si zaměření oborů, z nichž si budete vybírat. Orientovat se pak můžete podle zaměření fakult. Máte možnost zvolit si: ekonomické, pedagogické, filozofické, právnické, lékařské, umělecké, chemické, přírodovědné, technické, zemědělské.</p>

<h3>Vyberte si formu studia</h3>
<p>Chcete studovat prezenčně nebo kombinovaně? Rozhodněte o typu studia, které budete hledat. V nabídce fakult jsou zpravidla obory rozděleny podle typu studia na bakalářské nebo navazující magisterské. Na začátku si vybíráte obor bakalářský (3 nebo 4 roky studia), po složení zkoušky můžete pokračovat v navazujícím magisterském. Na některých školách se můžete hlásit přímo na dlouholeté  (5 nebo 6 let studia) magisterské obory.</p>

<h3>Zjistěte si podrobnosti a specifikujte svůj výběr</h3>
<p>O svém seznamu vybraných oborů si <b>povídejte s kýmkoliv</b>, kdo je ochoten naslouchat – rodiče, známí, přátelé. Jejich názor může být odlišný, ale získáte podněty k dalšímu přemýšlení.</p>
<p>Pak už stačí <b>sednout k internetu a najít si podrobnosti o oborech</b>, které vám v seznamu zůstaly. Popisy oborů jsou občas dost obecné, proto je dobré využít dny otevřených dveří a vyptat se na jejich obsah důsledně. </p>
<p><b>Zvažte také možnosti svého budoucího uplatnění</b>, odpovězte si na otázky: Najdete ve vystudovaném oboru uplatnění? Bude vaše práce ohodnocena tak, jak si představujete? Bude ohodnocena okamžitě nebo budete nejprve muset získat praxi? Budete ochotni čekat?</p>','admin','2012-11-7');
INSERT INTO clanky
VALUES(null,'Jak vybrat fakultu','Ve chvíli, kdy jste rozhodnuti o oboru, který chcete studovat, je na čase vybrat fakulty, na které podáte přihlášku.
        Existuje několik <b>základních kritérií, které berou uchazeči nejčastěji v úvahu</b>.','<h3>Dostupnost oboru</h3>
        <p><b>Na kterých fakultách lze vybraný obor studovat?</b> V NajdiVŠ je možné toto zjistit například fulltextovým vyhledáváním nebo pomocí klíčových slov. Hledat se vyplatí, protože např. anglistiku najdete nejen na filozofických fakultách, ale také na pedagogických; životní prostředí najdete na přírodovědeckých, zemědělských i chemických fakultách.</p>

        <h3>Šance na přijetí</h3>
        <p><b>Kolik zájemců přijmou?</b> Pro většinu uchazečů je šance na přijetí jedním s nejdůležitějších ukazatelů. Udává, kolik uchazečů, kteří podali přihlášku na danou fakultu, bylo přijato. Je ale dobré věnovat čas detailnějšímu zkoumání podmínek přijetí. Na jednotlivých fakultách mohou být přijímací zkoušky z různých předmětů nebo fakulta může nabízet bodové zvýhodnění, které je výhodné právě pro vás.</p>
        <p>Počty přihlášených a přijatých uchazečů v minulých letech můžete najít na webech fakult. Počty se většinou liší i u jednotlivých oborů</p>

        <h3>Místo</h3>
        <p><b>V jakém městě je fakulta?</b> Důležitou roli hraje dostupnost bydlení v daném městě. V současné době už nebývá problém s tím dostat se na koleje, nicméně jejich úroveň je velmi různá.</p>
        <p>Orientační ceny kolejí a ubytování v soukromí naleznete v porovnání fakult.</p>

        <h3>Náročnost studia</h3>
        <p><b>Chcete mít při studiu čas na brigády a další zájmy nebo chcete raději náročné studium, při kterém se hodně naučíte?</b> Tyto údaje na oficiálních stránkách fakulty většinou nenajdete. Můžete se zeptat známých, kamarádů nebo využít studentské diskusní weby.</p>

        <h3>Kvalita školy a výuky</h3>
        <p><b>Chcete prestižní školu s vysokou kvalitou výuky a dobrým jménem</b> nebo se spokojíte s nižším standardem, hlavně když to povede ke kýženému cíli? Jakou má škola prestiž napoví informace o tom, kolik uchazečů se na příslušnou fakultu hlásilo, kolik jich bylo přijato a kolik se jich skutečně zapsalo. Vypovídající je také způsob hodnocení studentů. Důležité je zjišťovat, zda by fakultu doporučili stávající studenti.</p>

        <h3>Uplatnění po studiu</h3>
        <p>V popisech oborů najdete většinou pouze obecně a široce definované možnosti uplatnění. Některé univerzity provádějí mezi svými absolventy pravidelné průzkumy (například Masarykova univerzita v Brně). K většině fakult však nejsou údaje o zaměstnání a platu absolventů k dispozici, obecně ale platí, že nezaměstnanost absolventů VŠ je nízká. Také zároveň platí, že zaměstnavatelé  už nechtějí slyšet pouze to, že máte titul, ale také, z jaké jej máte fakulty; úroveň a zaměření studia se může na jednotlivých fakultách poměrně lišit (některé fakulty se např. více orientují na vědecký výzkum, jiné se orientují na aplikaci znalostí do praxe…)</p>

        <h3>Přijímací zkoušky</h3>
        <p>Přijímací řízení je plně v kompetenci fakulty, která  tedy sama rozhoduje o tom,  na základě jakých kriterií budou uchazeče přijímat a kdy přijímací zkoušky proběhnou.</p>
        <p>Některé fakulty zrušily vlastní přijímací zkoušky a nahradily je jednotnými Národními srovnávacími zkouškami (NSZ). Některé fakulty také přijímají pouze na základě výsledků středoškolského studia.</p>
		<ul class="seznam">
            <li><b>Informace o přijímacím řízení najdete v přehledu fakult</a>.</b></li>
            <li><b>Přehled způsobu přijímaček najdete v části přijímací zkoušky</a>.</b></li>
        </ul>','admin','2012-11-9');

INSERT INTO clanky
VALUES(null,'Jak vyplnit přihlášku','Existují dvě formy přihlášky – papírová a elektronická. V posledních letech se na školách začala prosazovat tzv. elektronická přihláška. Pro ni na rozdíl od papírové neexistuje žádný jednotný formulář. S podáním přihlášky je většinou spojeno zaplacení poplatku za přijímací řízení.','<h3>Papírová přihláška</h3>
        <p>Formulář si můžete <a href="http://www.scio.cz/1_download/prihlaska_vs.pdf" target="_blank">stáhnout ve formátu pdf</a>, vyzvednout na vybrané fakultě, zakoupit lze často i v trafice.</p>
        <h3>Elektronická přihláška</h3>
        <p>Formuláře na vyplnění elektronických přihlášek jsou na stránkách jednotlivých fakult a jsou pro každou z nich rozdílné. I při vyplňování elektronické přihlášky se však často nevyhnete cestě na poštu. Nutná potvrzení, která fakulta požaduje, ve většině případů elektronicky poslat nelze.</p>
        <p>Jaké přihlášky jednotlivé fakulty přijímají, a jaké mají poplatky a co vše musíte dále spolu s příhláškou doložit, se dozvíte v <a href="/2-3-prehled-fakult.aspx">přehledu fakult</a>.</p>','admin','2012-11-22');

INSERT INTO clanky
VALUES(null,'Jak se připravit','Důležité je zjistit, co vlastně k přijímačkám na VŠ potřebujete. To se nejlépe dozvíte z internetových stránek vysokých škol. Většina fakult uvádí na svých stránkách i odkazy na testy použité u loňských přijímaček.','<p>V posledních letech stoupá počet fakult, které přijímají podle výsledků <a href="http://www.scio.cz/nsz" target="_blank">Národních srovnávacích zkoušek</a> společnosti Scio. Zkoušky se liší od klasických přijímaček zkoušek tím, že je možné jejich výsledek uplatnit k přijetí na více fakult VŠ.</p>
        <h2>Možnosti přípravy</h2>
        <h3>Nepovinné předměty a semináře</h3>
        <p>V rámci studia na střední škole máte možnost si volit různé nepovinné předměty či semináře. Pokud víte, jakým směrem se bude vaše další studium ubírat, volte si je podle toho. Stejně tak předměty, ze kterých budete maturovat.</p>
        <h3>Prezenční přípravné kurzy</h3>
        <p><b>…od fakult</b> – tyto přípravné kurzy se doporučují nejvíce. Seznámíte se navíc s prostředím i pedagogy a účast na přijímacích zkouškách pro vás nebude stresovou záležitostí. Některé fakulty nabízejí pro uchazeče o studium tzv. „<b>nulté ročníky</b>“.</p>
        <p><b>…od vzdělávacích agentur</b> – nabídka kurzů, které zaručeně na VŠ připraví, je velmi široká. Jejich kvalita je velmi různá. Pozor tedy na podvody, kdy vás za hodně peněz nic nenaučí. Ověřujte si kvalitu předem – sežeňte si doporučení absolventů, zjistěte si, na základě jakých materiálů bude výuka probíhat, jak velká bude skupina, jakou mají vyučující kvalifikaci.</p>
        <p><b>prezenční přípravné kurzy „na poslední chvíli“</b> - není možné se za pár dnů naučit celou matematiku či jiný obor, i kdyby to popis kurzu sliboval. Tyto „rychlokurzy“ vám spíše jen umožní si látku utřídit a zopakovat nejobtížnější místa těsně před zkouškami. Některé „rychlokurzy“ jsou pak zaměřené spíše na strategii řešení příslušného testu než-li na vysvětlování látky, která se v testu objeví.</p>
        <h3>Tištěné materiály</h3>
        <p>Publikací s přípravou k přijímacím zkouškám je velké množství. Vydávají je buď přímo fakulty, nebo různá jiná nakladatelství. Podobně jako u prezenčních kurzů, je lepší dávat přednost materiálům přímo od fakult, které přijímací testy připravují.</p>
        <h3>Příprava online</h3>
        <p>Některé firmy nabízejí online testy či internetové kurzy z různých oborů. Ty jsou určeny pro vaši samostatnou práci doma a většinou je možné komunikovat během studia s lektorem kurzu elektronickou cestou.</p>','admin','2012-11-22');

COMMIT;
