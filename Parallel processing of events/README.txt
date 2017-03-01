Nume: Ion Bogdan-Ionut
Grupa: 332CB




Pentru rezolvarea temei am folosit paradigma Producator-Consumator unde buffer-ul foloseste un ArrayBlockingQueue.
Am o clasa Event care reprezinta toate cele patru tipuri de evenimente si care contine metodele care calculeaza fiecare tip de eveniment. Folosesc metodele de put si take ale ArrayBlockingQueue-ului, unde put asteapta pentru spatiu in coada si take asteapta sa existe un element in coada.

Clasele EventGenerator si Worker implementeaza Runnable, deci voi avea cate un numar de thread-uri corespunzatoare fiecareia. EventGenerator  asteapta un timp, citeste linia dintr-un fisier, creeaza evenimentul asociat tipului specificat si il adauga in coada. Un worker preia evenimentul din coada, il prelucreaza si il adauga(sincronizat) in lista corespunzatoare tipului evenimentului.

In Main creez thread-uri EventGenerator si Worker, iar dupa ce si-au terminat treaba, sortez listele si scriu rezultatele in fisierele asociate celor patru tipuri de evenimente. Varianta de Java folosita este Java8.