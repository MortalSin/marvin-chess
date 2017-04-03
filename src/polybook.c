/*
 * Marvin - an UCI/XBoard compatible chess engine
 * Copyright (C) 2015 Martin Danielsson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "polybook.h"
#include "validation.h"
#include "utils.h"
#include "movegen.h"

/* Polyglot random numbers */
static const uint64_t poly_random64[781] = {
    11329126462075137345ULL, 3096006490854172103ULL, 4961560858198160711ULL,
    11247167491742853858ULL, 8467686926187236489ULL, 3643601464190828991ULL,
    1133690081497064057ULL, 16733846313379782858ULL, 972344712846728208ULL,
    1875810966947487789ULL, 10810281711139472304ULL, 14997549008232787669ULL,
    4665150172008230450ULL, 77499164859392917ULL, 6752165915987794405ULL,
    2566923340161161676ULL, 419294011261754017ULL, 7466832458773678449ULL,
    8379435287740149003ULL, 9012210492721573360ULL, 9423624571218474956ULL,
    17519441378370680940ULL, 3680699783482293222ULL, 5454859592240567363ULL,
    12278110483549868284ULL, 10213487357180498955ULL, 9786892961111839255ULL,
    1870057424550439649ULL, 13018552956850641599ULL, 8864492181390654148ULL,
    14503047275519531101ULL, 2642043227856860416ULL, 5521189128215049287ULL,
    1488034881489406017ULL, 12041389016824462739ULL, 236592455471957263ULL,
    7168370738516443200ULL, 707553987122498196ULL, 3852097769995099451ULL,
    8313129892476901923ULL, 1761594034649645067ULL, 2291114854896829159ULL,
    15208840396761949525ULL, 13805854893277020740ULL, 11490038688513304612ULL,
    5903053950100844597ULL, 6666107027411611898ULL, 18228317886339920449ULL,
    3626425922614869470ULL, 10120929114188361845ULL, 13383691520091894759ULL,
    9148094160140652064ULL, 1284939680052264319ULL, 7307368198934274627ULL,
    5611679697977124792ULL, 10869036679776403037ULL, 4819485793530191663ULL,
    7866624006794876513ULL, 4794093907474700625ULL, 6849775302623042486ULL,
    4177248038373896072ULL, 10648116955499083915ULL, 7195685255425235832ULL,
    17012007340428799350ULL, 6004979459829542343ULL, 575228772519342402ULL,
    5806056339682094430ULL, 8920438500019044156ULL, 1872523786854905556ULL,
    7168173152291242201ULL, 9388215746117386743ULL, 8767779863385330152ULL,
    1489771135892281206ULL, 17385502867130851733ULL, 15762364259840250620ULL,
    2649182342564336630ULL, 13505777571156529898ULL, 928423270205194457ULL,
    11861585534482611396ULL, 16833723316851456313ULL, 2875176145464482879ULL,
    9598842341590061041ULL, 6103491276194240627ULL, 8264435384771931435ULL,
    17191732074717978439ULL, 11134495390804798113ULL, 8118948727165493749ULL,
    17994305203349779906ULL, 9778408473133385649ULL, 11774350857553791160ULL,
    12559012443159756018ULL, 1810658488341658557ULL, 9781539968129051369ULL,
    658149708018956377ULL, 18376927623552767184ULL, 10225665576382809422ULL,
    11247233359009848457ULL, 12966474917842991341ULL, 4111328737826509899ULL,
    6628917895947053289ULL, 2166287019647928708ULL, 11129710491401161907ULL,
    5728850993485642500ULL, 7135057069693417668ULL, 2409960466139986440ULL,
    6600979542443030540ULL, 5794634036844991298ULL, 1765885809474863574ULL,
    7278670237115156036ULL, 16128398739451409575ULL, 17262998572099182834ULL,
    8877430296282562796ULL, 13401997949814268483ULL, 407550088776850295ULL,
    13080877114316753525ULL, 5365205568318698487ULL, 14935709793025404810ULL,
    17669982663530100772ULL, 4357691132969283455ULL, 17142609481641189533ULL,
    8763584794241613617ULL, 9679198277270145676ULL, 10941274620888120179ULL,
    11693142871022667058ULL, 306186389089741728ULL, 10524424786855933342ULL,
    8136607301146677452ULL, 8332101422058904765ULL, 6215931344642484877ULL,
    17270261617132277633ULL, 13484155073233549231ULL, 5040091220514117480ULL,
    10596830237594186850ULL, 18403699292185779873ULL, 12565676100625672816ULL,
    15937214097180383484ULL, 9145986266726084057ULL, 2521545561146285852ULL,
    14490332804203256105ULL, 9262732965782291301ULL, 16052069408498386422ULL,
    2012514900658959106ULL, 4851386166840481282ULL, 12292183054157138810ULL,
    12139508679861857878ULL, 7319524202191393198ULL, 16056131139463546102ULL,
    2445601317840807269ULL, 12976440137245871676ULL, 10500241373960823632ULL,
    1211454228928495690ULL, 2931510483461322717ULL, 14252799396886324310ULL,
    6217490319246239553ULL, 3253094721785420467ULL, 11224557480718216148ULL,
    17235000084441506492ULL, 12619159779355142232ULL, 5189293263797206570ULL,
    12606612515749494339ULL, 1850950425290819967ULL, 5933835573330569280ULL,
    17649737671476307696ULL, 1240625309976189683ULL, 13611516503114563861ULL,
    11359244008442730831ULL, 463713201815588887ULL, 5603848033623546396ULL,
    5837679654670194627ULL, 13869467824702862516ULL, 13001586210446667388ULL,
    12934789215927278727ULL, 2422944928445377056ULL, 3310549754053175887ULL,
    8519766042450553085ULL, 17839818495653611168ULL, 15503797852889124145ULL,
    16011257830124405835ULL, 862037678550916899ULL, 3197637623672940211ULL,
    5210919022407409764ULL, 14971170165545012763ULL, 12708212522875260313ULL,
    11160345150269715688ULL, 11888460494489868490ULL, 16669255491632516726ULL,
    7618258446600650238ULL, 17993489941568846998ULL, 18188493901990876667ULL,
    11270342415364539415ULL, 10288892439142166224ULL, 7423022476929853822ULL,
    14215600671451202638ULL, 8710936142583354014ULL, 18346051800474256890ULL,
    629718674134230549ULL, 10598630096540703438ULL, 10666243034611769205ULL,
    16077181743459442704ULL, 4303848835390748061ULL, 15183795910155040575ULL,
    17843919060799288312ULL, 15561328988693261185ULL, 15662367820628426663ULL,
    3706272247737428199ULL, 12051713806767926385ULL, 11742603550742019509ULL,
    5704473791139820979ULL, 9787307967224182873ULL, 1637612482787097121ULL,
    8908762506463270222ULL, 17556853009980515212ULL, 4157033003383749538ULL,
    18207866109112763428ULL, 1800584982121391508ULL, 5477894166363593411ULL,
    4674885479076762381ULL, 10160025381792793281ULL, 7550910419722901151ULL,
    8799727354050345442ULL, 11321311575067810671ULL, 4039979115090434978ULL,
    3605513501649795505ULL, 3876110682321388426ULL, 12180869515786039217ULL,
    8620494007958685373ULL, 5854220346205463345ULL, 4855373848161890066ULL,
    15654983601351599195ULL, 5949110547793674363ULL, 5957016279979211145ULL,
    11321480117988196211ULL, 8228060533160592200ULL, 2094843038752308887ULL,
    8801329274201873314ULL, 297395810205168342ULL, 6489982145962516640ULL,
    925952168551929496ULL, 6268205602454985292ULL, 2903841526205938350ULL,
    359914117944187339ULL, 8371662176944962179ULL, 11139146693264846140ULL,
    9807576242525944290ULL, 5795683315677088036ULL, 12688959799593560697ULL,
    1070089889651807102ULL, 6778454470502372484ULL, 17760055623755082862ULL,
    1983224895012736197ULL, 15760908081339863073ULL, 942692161281275413ULL,
    12134286529149333529ULL, 10647676541963177979ULL, 11090026030168016689ULL,
    5245566602671237210ULL, 9195060651485531055ULL, 6368791473535302177ULL,
    3229483537647869491ULL, 15232282204279634326ULL, 928484295759785709ULL,
    1909608352012281665ULL, 10412093924024305118ULL, 5773445318897257735ULL,
    3990834569972524777ULL, 10771395766813261646ULL, 4209783265310087306ULL,
    15318153364378526533ULL, 616435239304311520ULL, 17961392050318287288ULL,
    7798983577523272147ULL, 3913469721920333102ULL, 15424667983992144418ULL,
    6239239264182308800ULL, 1654244791516730287ULL, 17228895932005785491ULL,
    6221161860315361832ULL, 17056602083001532789ULL, 13458912522609437003ULL,
    12917665617485216338ULL, 7337288846716161725ULL, 13022188282781700578ULL,
    12979943748599740071ULL, 510457344639386445ULL, 8796640079689568245ULL,
    13565008864486958290ULL, 6465331256500611624ULL, 11031297210088248644ULL,
    8017026739316632057ULL, 3627975979343775636ULL, 15052215649796371267ULL,
    6222903725779446311ULL, 3527832623857636372ULL, 15597050972685397327ULL,
    8924250025456295612ULL, 14400806714161458836ULL, 10699110515857614396ULL,
    14468157413083537247ULL, 4223238849618215370ULL, 15681850266533497060ULL,
    1140009269240963018ULL, 12966521765762216121ULL, 12695701950206930564ULL,
    3881319844097050799ULL, 16858671235974049358ULL, 17004178443650550617ULL,
    10544522896658866816ULL, 13378871666599081203ULL, 7580967567056532817ULL,
    14279886347066493375ULL, 14791316027199525482ULL, 13540141887354822347ULL,
    15889873206108611120ULL, 13441296750672675768ULL, 11798467976251859403ULL,
    16858792058461978657ULL, 704784010218719535ULL, 9596982322589424841ULL,
    9297677921824001878ULL, 687173692492309888ULL, 2573542046251205823ULL,
    14064986013008197277ULL, 5122261027125484554ULL, 12166444546397347981ULL,
    392580029432520891ULL, 13077660124902070727ULL, 16778702188287612735ULL,
    3451078315256158032ULL, 1238907336018749328ULL, 9205113463181886956ULL,
    1667962162104261376ULL, 10830753981784044039ULL, 4479827962372740717ULL,
    13723669708721922220ULL, 17895945165757891767ULL, 5275192813757817777ULL,
    2148246364622112874ULL, 2290795724393258885ULL, 18193581350273252090ULL,
    1776293542351822525ULL, 14757011774120772237ULL, 4313244667902787366ULL,
    12281515972708701602ULL, 16810874891151093887ULL, 13231770820477907822ULL,
    15338037979535853741ULL, 3321611548688927336ULL, 3305807524324674332ULL,
    13385011844708802686ULL, 7248312053715383136ULL, 10692263740491040132ULL,
    15834887971838928217ULL, 15164530629649278767ULL, 9112428691881135949ULL,
    7848957776938116907ULL, 10951816186743012388ULL, 8896660382367628050ULL,
    9603906275513256852ULL, 8762207035762213579ULL, 14987444343672838948ULL,
    9409751230138127831ULL, 10591026249259463665ULL, 7197363620976276483ULL,
    14301381657157454364ULL, 6373588016705149671ULL, 685071415365890925ULL,
    11485719029193745472ULL, 11525714121369126191ULL, 16463451990009075596ULL,
    16713578179004591821ULL, 6251124536988276734ULL, 6144308296388004591ULL,
    8880818733894805775ULL, 1303007271453773655ULL, 9174156641096830119ULL,
    8824404812019774483ULL, 4420129794615782201ULL, 9951556838786075828ULL,
    8883975763174874978ULL, 10736884308676275715ULL, 5595889224692918441ULL,
    4306406647446967767ULL, 6704191827946442961ULL, 9195534799547011879ULL,
    15724940538984617905ULL, 15915014237009546277ULL, 3928039610514994951ULL,
    14873195079178728329ULL, 12362539403674935092ULL, 4869881251581666789ULL,
    12986343614603388393ULL, 1215083005313393810ULL, 15835354158744478399ULL,
    11186056805483324290ULL, 13149236123055901828ULL, 13821214860367539280ULL,
    12182689304549523133ULL, 2305696533800337221ULL, 12399248800711438055ULL,
    12612571074767202621ULL, 1949121388445288260ULL, 13067734303660960050ULL,
    14085928898807657146ULL, 14099042149407050217ULL, 17561987301945706495ULL,
    11512458344154956250ULL, 7437568954088789707ULL, 7915171836405846582ULL,
    11752651295154297649ULL, 520574178807700830ULL, 9984063241072378277ULL,
    16254155646211095029ULL, 8412807604418121470ULL, 5609875541891257226ULL,
    11323858615586018348ULL, 8376971840073549054ULL, 1383314287233606303ULL,
    15474222835752021056ULL, 5204145074798490767ULL, 2167677454434536938ULL,
    10341418833443722943ULL, 8271005071015654673ULL, 15537457915439920220ULL,
    10730891177390075310ULL, 11511496483171570656ULL, 16026237624051288806ULL,
    11839117319019400126ULL, 11321351259605636133ULL, 5895970210948560438ULL,
    3447475526873961356ULL, 7334775646005305872ULL, 15954460007382865005ULL,
    6939292427400212706ULL, 8334626163711782046ULL, 1912937584935571784ULL,
    12304971244567641760ULL, 8524679326357320614ULL, 2204997376562282123ULL,
    3197166419597805379ULL, 4220875528993937793ULL, 2803169229572255230ULL,
    5085503808422584221ULL, 14444799216525086860ULL, 4570145336765972565ULL,
    9186432380899140933ULL, 11239615222781363662ULL, 9872907954749725788ULL,
    10369691348610460342ULL, 11573842626212501214ULL, 18049927275724560211ULL,
    15471783285232223897ULL, 16134745906572777443ULL, 13149419803421182712ULL,
    14564139292183438565ULL, 2088698177441502777ULL, 15099871677732932330ULL,
    5679318949880730421ULL, 16491038769688081874ULL, 1684901764271550206ULL,
    6019498834983443029ULL, 8308552077872645018ULL, 2774412133178445207ULL,
    2993471197969887147ULL, 8756104692490586069ULL, 7404378077533100169ULL,
    11391825116471223489ULL, 17128408637045999621ULL, 5816122712455824169ULL,
    5531291136777113635ULL, 7400684525794093602ULL, 2421696223438995901ULL,
    2746718911238191773ULL, 2297623779240041360ULL, 15514986454711725499ULL,
    13355177993350187464ULL, 2151598180055853022ULL, 14933732441462847914ULL,
    17651243408385815107ULL, 4086544267540179726ULL, 3960368502933186560ULL,
    16948614951473504462ULL, 11262612224635188739ULL, 12613511070148831882ULL,
    2706199935239343179ULL, 10054459213633325149ULL, 17640957734094436437ULL,
    15290986714861486531ULL, 16616573458614039565ULL, 2626432152093131908ULL,
    14024745482209308341ULL, 12344195406125417964ULL, 7167044992416702836ULL,
    11933989054878784040ULL, 1255659969011027721ULL, 3240842176865726111ULL,
    795178308456769763ULL, 12389083385239203825ULL, 6408553047871587981ULL,
    14331996049216472800ULL, 3362936192376505047ULL, 1486633608756523830ULL,
    8937438391818961808ULL, 15513702763578092231ULL, 9242607645174922067ULL,
    16999375738341892551ULL, 225631029947824688ULL, 5294122026845313316ULL,
    11666909141406975304ULL, 6576914768872977647ULL, 13014342141693467190ULL,
    15296769519938257969ULL, 1344590668019013826ULL, 8870296219354404ULL,
    1763076921063072981ULL, 11710831831040350446ULL, 11042296215092253456ULL,
    12923501896423220822ULL, 2679459049130362043ULL, 15149139477832742400ULL,
    2006921612949215342ULL, 2441159149980359103ULL, 4254066403785111886ULL,
    10165995291879048302ULL, 17968517685540419316ULL, 4067155115498534723ULL,
    14584673823956990486ULL, 7262306400971602773ULL, 2599246507224983677ULL,
    1183331494191622178ULL, 9203696637336472112ULL, 8684305384778066392ULL,
    452576500022594089ULL, 7158260433795827572ULL, 5749101480176103715ULL,
    2141838636388669305ULL, 13319697665469568251ULL, 11739738846189583585ULL,
    15704600611932076809ULL, 17288566729036156523ULL, 3345333136360207999ULL,
    12225668941959679643ULL, 13135848755558586049ULL, 8127707564878445808ULL,
    11020438739076919854ULL, 13800233257954351967ULL, 10719452353263111411ULL,
    4467639418469323241ULL, 13341252870622785523ULL, 7043015398453076736ULL,
    13802777531561938248ULL, 2597087673064131360ULL, 18196619797102886407ULL,
    17222554220133987378ULL, 11603572837337492490ULL, 9373650498706682568ULL,
    15247985213323458255ULL, 2826050093225892884ULL, 7047939442312345917ULL,
    1975862676241125979ULL, 8471065344236531211ULL, 10781433328192619353ULL,
    12710259184248419661ULL, 6983092299355911633ULL, 8891398163252015007ULL,
    18232837537224201402ULL, 10128874404256367960ULL, 1184291664448112016ULL,
    8752186474456668498ULL, 11883874832968622155ULL, 8304258407043758711ULL,
    13031437632736158055ULL, 11394657882570178521ULL, 11346359947151974253ULL,
    15207539437603825135ULL, 6743071165850287963ULL, 1895531807983368793ULL,
    8070015023023620019ULL, 15994912017468668362ULL, 7264555371116116147ULL,
    638838107884199779ULL, 612060626599877907ULL, 16368581545287660539ULL,
    2028126038944990910ULL, 8217932366665821866ULL, 12715716898990721499ULL,
    4917760284400488853ULL, 4689038209317479950ULL, 15570055495392019914ULL,
    7353589116749496814ULL, 6461588461223219363ULL, 16737230234434607639ULL,
    10643751583066909176ULL, 13889371344374910415ULL, 14623784806974468748ULL,
    6280119077769544053ULL, 5795026310427216669ULL, 15581542564775929183ULL,
    5344020438314994897ULL, 17090582320435646615ULL, 13070392342864893666ULL,
    2499216570383001617ULL, 5973851566933180981ULL, 11163195574208743088ULL,
    10686881252049739702ULL, 7802414647854227001ULL, 7696730671131205892ULL,
    11939552629336260711ULL, 8954801150602803298ULL, 5805966293032425995ULL,
    10608482480047230587ULL, 4997389530575201269ULL, 7710978612650642680ULL,
    7716832357345836839ULL, 15123312752564224361ULL, 16000314919358148208ULL,
    5766400084981923062ULL, 11245886267645737076ULL, 8713884558928322285ULL,
    7910921931260759656ULL, 17192478743862940141ULL, 3651028258442904531ULL,
    4208705969817343911ULL, 3568641929344250749ULL, 7493701010274154640ULL,
    2245920858524015772ULL, 13159017457951468389ULL, 12290633441485835508ULL,
    17599068061438200851ULL, 18107352842948477138ULL, 3841784002685309084ULL,
    3972025232192455038ULL, 7780701379940603769ULL, 14773200954226001784ULL,
    16368109790951669962ULL, 11498059885876068682ULL, 331717439817162336ULL,
    18209951341142539931ULL, 639100052003347099ULL, 10347169565922244001ULL,
    13093097841025825382ULL, 2526013881820679475ULL, 4894708394808468861ULL,
    4217798054095379555ULL, 2415982786774940751ULL, 2008219703699744969ULL,
    6034935405124924712ULL, 16377935039880138091ULL, 15469949637801139582ULL,
    6813989660423069229ULL, 3171782229498906237ULL, 12757488664123869734ULL,
    4587441767303016857ULL, 1011542511767058351ULL, 1218420902424652599ULL,
    11452069637570869555ULL, 15332250653395824223ULL, 9318912313336593440ULL,
    10499356348280572422ULL, 17042034373048666488ULL, 1805505087651779950ULL,
    13083730121955101027ULL, 9926866826056072641ULL, 12395083137174176754ULL,
    13014086693993705056ULL, 18092419734315653769ULL, 4496402702769466389ULL,
    4275128525646469625ULL, 16718947186147009622ULL, 2644524053331857687ULL,
    16665345306739798209ULL, 756689505943647349ULL, 6332958748006341455ULL,
    5397518675852254155ULL, 3282372277507744968ULL, 15124857616913606283ULL,
    9958173582926173484ULL, 550475751710050266ULL, 9535384695938759828ULL,
    11027794851313865315ULL, 1895999114042080393ULL, 17795970715748483584ULL,
    3512907883609256988ULL, 10170876972722661254ULL, 5100888107877796098ULL,
    14766188770308692257ULL, 5664728055166256274ULL, 1867780161745570575ULL,
    5069314540135811628ULL, 10826357501146152497ULL, 8428576418859462269ULL,
    6489498281288268568ULL, 248384571951887537ULL, 14408891171920865889ULL,
    3830179243734057519ULL, 10976374785232997173ULL, 12375273678367885408ULL,
    14917570089431431088ULL, 5317296011783481118ULL, 8812437177215009958ULL,
    15702128452263965086ULL, 1418237564682130775ULL, 8287918193617750527ULL,
    5641726496814939044ULL, 18399300296243087930ULL, 6176181444192939950ULL,
    13286219625023629664ULL, 14609847597738937780ULL, 15778618041730427743ULL,
    13113915167160321176ULL, 3534397173597697283ULL, 16753315048725296654ULL,
    2378655170733740360ULL, 17894101054940110861ULL, 551298419243755034ULL,
    14177640314441820846ULL, 18011171644070679608ULL, 1942137629605578202ULL,
    17704970308598820532ULL, 10820688583425137796ULL, 319261663834750185ULL,
    17320020179565189708ULL, 10828766552733203588ULL, 11254165892366229437ULL,
    5921710089078452638ULL, 1692791583615940497ULL, 3154220012138640370ULL,
    2462272376968205830ULL, 5215882904155809664ULL, 9063345109742779520ULL,
    10012495044321978752ULL, 2282028593076952567ULL, 16490284710305269338ULL,
    11358175869672944140ULL, 2648366387851958704ULL, 2535530668932196013ULL,
    15386192992268326902ULL, 6797681746413993003ULL, 9131737009282615627ULL,
    744965241806492274ULL, 15534171479957703942ULL, 11406512201534848823ULL,
    1724859165393741376ULL, 2131804225590070214ULL, 10649852818715990109ULL,
    7348272751505534329ULL, 15418610264624661717ULL, 14030296408486517359ULL,
    6426639016335384064ULL, 14857241317133980380ULL, 8982836549816060296ULL,
    2847738978322528776ULL, 14275200949057556108ULL, 1517491100508351526ULL,
    11487065943069529588ULL, 7252270709068430025ULL, 1454069630547688509ULL,
    879136823698237927ULL, 764541931096396549ULL, 16628452526739142958ULL,
    8210570252116953863ULL, 17419012767447246106ULL, 16656819168530874484ULL,
    10879562253146277412ULL, 9340840147615694245ULL, 6892625624787444041ULL,
    6239858431661771035ULL, 10484131262376733793ULL, 15135908441777759839ULL,
    3591372000141165328ULL, 17394508730963952016ULL, 11925077963498648480ULL,
    2231224496660291273ULL, 8127998803539291684ULL, 16292452481085749975ULL,
    16488107566197090ULL, 2060923303336906913ULL, 14929791059677233801ULL,
    15052228947759922034ULL, 8630622898638529667ULL, 7467898009369859339ULL,
    17930561480947107081ULL
};
#define RANDOM_PIECE(k, f, r)   poly_random64[64*(k)+8*(r)+(f)]
#define RANDOM_CASTLE(c)        poly_random64[768+(c)]
#define RANDOM_EN_PASSANT(f)    poly_random64[772+(f)]
#define RANDOM_TURN()           poly_random64[780]

/* Polyglot piece definitions */
#define POLY_BLACK_PAWN     0
#define POLY_WHITE_PAWN     1
#define POLY_BLACK_KNIGHT   2
#define POLY_WHITE_KNIGHT   3
#define POLY_BLACK_BISHOP   4
#define POLY_WHITE_BISHOP   5
#define POLY_BLACK_ROOK     6
#define POLY_WHITE_ROOK     7
#define POLY_BLACK_QUEEN    8
#define POLY_WHITE_QUEEN    9
#define POLY_BLACK_KING     10
#define POLY_WHITE_KING     11

/* Polyglot castling definitions */
#define WHITE_CASTLE_SHORT  0
#define WHITE_CASTLE_LONG   1
#define BLACK_CASTLE_SHORT  2
#define BLACK_CASTLE_LONG   3

/* The size on disk of an opening book entry (in bytes) */
#define BOOK_ENTRY_SIZE 16

/* An opening book entry in Polyglot format */
struct polybook_entry {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
};

static FILE *bookfp = NULL;
static long booksize = 0;

static uint8_t engine2poly_piece(uint8_t piece)
{
    if ((piece&0x01) == 0) {
        return piece + 1;
    } else {
        return piece - 1;
    }
}

static uint8_t engine2poly_enpassant(struct gamestate *pos, uint8_t ep)
{
    int sq;
    int file;

    if (pos->stm == WHITE) {
        sq = ep - 8;
    } else {
        sq = ep + 8;
    }
    file = FILENR(sq);
    if ((file > 0) && (pos->pieces[sq-1] == (PAWN+pos->stm))) {
        return file;
    } else if ((file < 7) && (pos->pieces[sq+1] == (PAWN+pos->stm))) {
        return file;
    }
    return NFILES;
}

static uint8_t engine2poly_turn(uint8_t side)
{
    return side == WHITE?1:0;
}

static uint32_t poly2engine_move(struct gamestate *pos, struct movelist *list,
                                 uint16_t polymove)
{
    uint32_t move;
    int      k;
    int      from;
    int      to;
    int      promotion;

    to = SQUARE(polymove&0x0007, (polymove>>3)&0x0007);
    from = SQUARE((polymove>>6)&0x0007, (polymove>>9)&0x0007);
    promotion = ((polymove>>12)&0x0007)*2 + pos->stm;

    for (k=0;k<list->nmoves;k++) {
        move = list->moves[k];
        if ((TO(move) == to) && (FROM(move) == from)) {
            if ((promotion >= WHITE_KNIGHT) && (PROMOTION(move) == promotion)) {
                return move;
            } else if (promotion < WHITE_KNIGHT) {
                return move;
            }
        }
    }

    return NOMOVE;
}

static uint64_t generate_polykey(struct gamestate *pos)
{
    uint64_t key;
    int      k;
    int      poly_piece;
    int      poly_ep;
    int      poly_turn;

    key = 0ULL;

    /* Add pieces */
    for (k=0;k<NSQUARES;k++) {
        if (pos->pieces[k] == NO_PIECE) {
            continue;
        }
        poly_piece = engine2poly_piece(pos->pieces[k]);
        key ^= RANDOM_PIECE(poly_piece, FILENR(k), RANKNR(k));
    }

    /* Add en-passant target square */
    if (pos->ep_sq != NO_SQUARE) {
        poly_ep = engine2poly_enpassant(pos, pos->ep_sq);
        if (poly_ep != NFILES) {
            key ^= RANDOM_EN_PASSANT(poly_ep);
        }
    }

    /* Add castling permissions */
    if (pos->castle&WHITE_KINGSIDE) {
        key ^= RANDOM_CASTLE(WHITE_CASTLE_SHORT);
    }
    if (pos->castle&WHITE_QUEENSIDE) {
        key ^= RANDOM_CASTLE(WHITE_CASTLE_LONG);
    }
    if (pos->castle&BLACK_KINGSIDE) {
        key ^= RANDOM_CASTLE(BLACK_CASTLE_SHORT);
    }
    if (pos->castle&BLACK_QUEENSIDE) {
        key ^= RANDOM_CASTLE(BLACK_CASTLE_LONG);
    }

    /* Add side to move */
    poly_turn = engine2poly_turn(pos->stm);
    if (poly_turn != 0) {
        key ^= RANDOM_TURN();
    }

    return key;
}

static bool read_book_entry(long offset, struct polybook_entry *entry)
{
    uint8_t buffer[16];
    size_t  nbytes;

    if (fseek(bookfp, offset, SEEK_SET) != 0) {
        return false;
    }

    nbytes = fread(buffer, 1, 16, bookfp);
    if (nbytes != 16) {
        return false;
    }

    entry->key = read_uint64(buffer);
    entry->move = read_uint16(buffer+8);
    entry->weight = read_uint16(buffer+10);
    entry->learn = read_uint32(buffer+12);

    return true;
}

static struct book_entry* read_book_entries(struct gamestate *pos,
                                            uint64_t polykey, long index,
                                            int *count)
{
    long                  nentries;
    struct polybook_entry polyentry;
    struct book_entry     *entries;
    struct book_entry     *entry;
    struct movelist       list;

    gen_legal_moves(pos, &list);

    *count = 0;
    entries = malloc(MAX_MOVES*sizeof(struct book_entry));
    nentries = booksize/BOOK_ENTRY_SIZE;
    while (index < nentries) {
        if (!read_book_entry(index*BOOK_ENTRY_SIZE, &polyentry)) {
            *count = 0;
            free(entries);
            return NULL;
        }
        if (polyentry.key != polykey) {
            break;
        }

        entry = &entries[*count];
        entry->weight = polyentry.weight;
        entry->move = poly2engine_move(pos, &list, polyentry.move);

        if (entry->move != NOMOVE) {
            (*count)++;
        }
        index++;
    }

    return entries;
}

static uint32_t select_book_move(struct book_entry *entries, int nentries)
{
    int      k;
    uint32_t sum;
    uint32_t r;

    sum = 0;
    for (k=0;k<nentries;k++) {
        sum += entries[k].weight;
    }

    r = rand()%sum;
    sum = 0;
    for (k=0;k<nentries;k++) {
        sum += entries[k].weight;
        if (r < sum) {
            return entries[k].move;
        }
    }
    return entries[nentries-1].move;
}

bool polybook_open(char *path)
{
    assert(path != NULL);

    bookfp = fopen(path, "rb");
    if (bookfp == NULL) {
        return false;
    }

    if (fseek(bookfp, 0, SEEK_END) != 0) {
        polybook_close();
        return false;
    }
    booksize = ftell(bookfp);
    if (booksize == -1) {
        polybook_close();
        return false;
    }

    return true;
}

void polybook_close(void)
{
    if (bookfp != NULL) {
        fclose(bookfp);
        bookfp = NULL;
        booksize = 0;
    }
}

/*
 * A description of the opening book format can be found at:
 * http://hgm.nubati.net/book_format.html
 */
uint32_t polybook_probe(struct gamestate *pos)
{
    uint64_t              polykey;
    struct polybook_entry polyentry;
    struct book_entry     *entries;
    int                   nentries;
    bool                  found;
    long                  left;
    long                  right;
    long                  center;
    long                  index;
    uint32_t              move;

    assert(valid_board(pos));

    if (bookfp == NULL) {
        return NOMOVE;
    }

    /* Generate a Polyglot key */
    polykey = generate_polykey(pos);

    /* Use a binary search to find an entry with the correct key */
    found = false;
    center = 0;
    left = 0;
    right = booksize/BOOK_ENTRY_SIZE;
    while (left < right) {
        center = (left + right)/2;
        if (!read_book_entry(center*BOOK_ENTRY_SIZE, &polyentry)) {
            return NOMOVE;
        }

        if (polyentry.key == polykey) {
            found = true;
            break;
        } else if (polyentry.key > polykey) {
            right = center;
        } else if (polyentry.key < polykey) {
            left = center + 1;
        }
    }
    if (!found) {
        return NOMOVE;
    }

    /* Find the first entry with the same key */
    index = center - 1;
    while (index >= 0) {
        if (!read_book_entry(index*BOOK_ENTRY_SIZE, &polyentry)) {
            return NOMOVE;
        }
        if (polyentry.key != polykey) {
            break;
        }
        index--;
    }
    if (polyentry.key != polykey) {
        index++;
    }

    /* Read all book entries */
    entries = read_book_entries(pos, polykey, index, &nentries);
    if ((entries == NULL) || (nentries == 0)) {
        free(entries);
        return NOMOVE;
    }

    /* Select a move from the book entries at random */
    move = select_book_move(entries, nentries);

    /* Clean up */
    free(entries);

    return move;
}

/*
 * A description of the opening book format can be found at:
 * http://hgm.nubati.net/book_format.html
 */
struct book_entry* polybook_get_entries(struct gamestate *pos, int *nentries)
{
    uint64_t              polykey;
    struct polybook_entry polyentry;
    bool                  found;
    long                  left;
    long                  right;
    long                  center;
    long                  index;

    assert(valid_board(pos));

    if (bookfp == NULL) {
        return NOMOVE;
    }

    /* Generate a Polyglot key */
    polykey = generate_polykey(pos);

    /* Use a binary search to find an entry with the correct key */
    found = false;
    center = 0;
    left = 0;
    right = booksize/BOOK_ENTRY_SIZE;
    while (left < right) {
        center = (left + right)/2;
        if (!read_book_entry(center*BOOK_ENTRY_SIZE, &polyentry)) {
            return NOMOVE;
        }

        if (polyentry.key == polykey) {
            found = true;
            break;
        } else if (polyentry.key > polykey) {
            right = center;
        } else if (polyentry.key < polykey) {
            left = center + 1;
        }
    }
    if (!found) {
        return NOMOVE;
    }

    /* Find the first entry with the same key */
    index = center - 1;
    while (index >= 0) {
        if (!read_book_entry(index*BOOK_ENTRY_SIZE, &polyentry)) {
            return NOMOVE;
        }
        if (polyentry.key != polykey) {
            break;
        }
        index--;
    }
    if (polyentry.key != polykey) {
        index++;
    }

    return read_book_entries(pos, polykey, index, nentries);
}