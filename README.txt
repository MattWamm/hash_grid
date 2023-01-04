welkom bij mijn creatie!

mijn project is veranderd in structuur door een spatial hash grid toe te passen die ik vond in mijn onderzoek
als de beste manier van het oplossen van de collision
de collision was de grootste offender destijds door 60% van alle tijd op te nemen in update

de big O staat achter alle for() statements in Game::update() (de originele big o niet de nieuwe)

op de spatial hash grid heb ik meeste van mijn verbeteringen gebasseerd

vooral collision checks zijn makkelijk te verbeteren aangezien alle collisions te maken hebben met de tanks behalve de convex hull

de sort_tanks_health heb ik vervangen met een merge sort

ik heb samen met robert geprobeerd een a* implementatie te schrijven maar door een serie van onbegrijpelijke bugs heb ik het opgegeven
om 11:45 op de inlever datum.
ik weet hoe a* werkt maar zelfs als onze implementaties hetzelfde waren werkte het niet.

ook heb ik een simpel algoritme geschreven in de closest_enemy code die 2 keer door de grid heen loopt zodat het niet 4000 tanks moet
checken maar 200 cells en dan bij paar die dichtbij genoeg zijn misschien 100 per cell

de grootste tijd kost is nu draw():20% en get_route():20%

ik heb de threadpool geprobeerd te implementeren op verschillende locaties met negatieve resultaten
de overhead was simpelweg te groot om te gebruiken en ik wist niet hoe ik dat moest oplossen
ik wou liever een hoge speedup hebben
en door de korte tijd die ik nog over had wou ik geen tijd verspillen aan iets dat ik wist niet ging werken.

^
van vorige keer

ik heb hier nog aan toegevoegd dat ik de threadpool heb aangepast naarmate van wat onderzoek op internet
hierdoor kon ik beter begrijpen hoe ik het moest gebruiken
ik heb een paar dingen in de main loop aangepast en gemultithread
maar doordat mijn algoritme een specifieke functie heeft die op onbegrijpelijke wijze altijd een read access violation gooit
zelfs als ik het uitvoer nadat alles wat er mee te maken heeft klaar is.

hiervoor heb ik een grote functie aan de top gemultithread met een klein resultaat en een andere functie in het midden.
