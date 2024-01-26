extends Control
var max_energy = 3
var energy = 3
var number_of_cards = 10

signal signal_ok

var full_room:bool = false
var yourTurn:bool = false
var can_play_modificator:bool = false
var deck = []
var drawPile = []
var discardPile = []
var playerHand  = []
var monsters = []
var clasNum = 6
var clas = ["Bard","Mag","Hunter","Guard","Warrior","Rogue"]
var clasCounter = {
	"Warrior": 0,
	"Guard":0,
	"Hunter": 0,
	"Rogue": 0,
	"Mag": 0,
	"Bard": 0
}
#Presets
var playableCard = preload("res://playable_card.tscn")
var heroCardView = preload("res://Prefabs/played_hero_view.tscn")
var monsterCard = preload("res://monster_card.tscn")
var monsterCardViewBtn = preload("res://Prefabs/monster_card_btn.tscn")

#opcje sieciowe
var HOST: String = Globals.HOST
var PORT: int = int(Globals.PORT)
const RECONNECT_TIMEOUT: float = 3.0

const Client = preload("res://client.gd")
var _client: Client = Client.new()
#END opcji sieciowych


# Called when the node enters the scene tree for the first time.
func _ready():
	#opcje sieciowe
	#_client.connect("connected", self, "_handle_client_connected")
	_client.connect("disconnected",_handle_client_disconnected)
	#_client.connect("error", self, "_handle_client_error")
	_client.connect("data", _handle_client_data)
	add_child(_client)
	#END opcji sieciowych

func _process(delta):
	if Input.is_action_just_pressed("ui_cancel"):
		$OptionPanel.visible = !$OptionPanel.visible

#Funkcje Sieciowe
func _connect_after_timeout(timeout: float) -> void:
	#yield(get_tree().create_timer(timeout), "timeout") # Delay for timeout
	_client.connect_to_host(HOST, PORT)

func _handle_client_connected() -> void:
	print("Client connected to server.")

func _handle_client_disconnected() -> void:
	if(!full_room):
		get_tree().change_scene_to_file("res://Scenes/serwer_disconect.tscn")
	else:
		full_room = false

func _handle_client_data(data: String) -> void:
	var msgArray = data.split(":")
	match msgArray[1]:
		"Monsters":
			print("Monsters:")
			var monsters = msgArray[0].split(",")
			var id = int(msgArray[2])
			if id!=0:
				for m in $Monsters.get_children():
					if m.idMonster==id:
						m.queue_free()
				for m in $CenterContainer/MonsterView/HBoxContainer.get_children():
					if m.idMonster==id:
						m.queue_free()
			for monster in monsters:
				addMonster(int(monster))
		"Leader":
			print("leader:")
			var id = int(msgArray[0])
			print("leader ID:")
			print(id)
			var link = str(id)+".jpg"
			$LeaderCard.texture = load("res://Cards/Liderzy/"+link)
			id -= 1
			changeHeroClass(clas[id],1)
			var name = "Energy"+clas[id]+".png"
			$EnergyBall.texture = load("res://Images/Energy/"+name)
		"Hand":
			print("Hand:")
			var cards = msgArray[0].split(",")
			for card in cards:
				addCardToHand(int(card))
			signal_ok.emit()
		"Card":
			print("Card:")
			var cards = msgArray[0].split(",")
			energy-=1
			$EnergyBall.changeEnergy(energy)
			for card in cards:
				addCardToHand(int(card))
		"Played":
			var id = int(msgArray[0])
			var node = heroCardView.instantiate()
			node.set_icon(id)
			$ScrollContainer3/PlayedHeroesOp.add_child(node)
		"DiceForMonster":
			print("Dice for Monster")
			energy-=2
			$EnergyBall.changeEnergy(energy)
			$CenterContainer/MonsterView.hide()
			$DiceValPanel/AcceptDiceVal.show()
			$DiceValPanel.show()
			%DiceVal.text = msgArray[0]
			can_play_modificator = true
		"DiceForMonsterOp":
			$DiceValPanel/AcceptDiceVal.hide()
			$DiceValPanel.show()
			%DiceVal.text = msgArray[0]
			can_play_modificator = true
		"NewDiceValForMonster":
			%DiceVal.text = msgArray[0]
			signal_ok.emit()
		"NmRSM":
			show_msg_box("NIE MOŻESZ WYZWAĆ TEGO POTWORA!!!")
		"Slayed":
			can_play_modificator = false
			$DiceValPanel.hide()
			var sl = int(msgArray[0])
			if sl!=0:
				var node = monsterCard.instantiate()
				node.set_icon(sl)
				$SlayedMonsters.add_child(node)
			else:
				show_msg_box("NIE UDAŁO CI SIĘ ZABIĆ TEGO POTWORA")
		"NotInterupt":
			can_play_modificator = false
			$DiceValPanel.hide()
		"Turn":
			turn(int(msgArray[0]))
		"OK":
			print("jest git!")
			signal_ok.emit()
		"GameStart": # sygnał rozpoczęci gry
			waitForStartResources()
		"OpponentDis": # przeciwnik się rozłączył
			get_tree().change_scene_to_file("res://Scenes/opponent_disconect.tscn")
		"WIN":
			Globals.winCondition = int(msgArray[0])
			get_tree().change_scene_to_file("res://Scenes/win.tscn")
		"LOSE":
			get_tree().change_scene_to_file("res://Scenes/lose.tscn")
		"FullRoom":
			full_room = true
			$ConnectPanel/VBoxContainer/Label2.text = "Ten pokój jest pełen"
		_:
			print("Błędny komunikat")


func _handle_client_error() -> void:
	print("Client error.")
	_connect_after_timeout(RECONNECT_TIMEOUT) # Try to reconnect after 3 seconds

#END Funkcje Sieciowe

func _drawCardAction():
	if can_play_modificator:
		show_msg_box("NAJPIERW WALKA !!!")
		return
	if !yourTurn:
		show_msg_box("NIE TWOJA TURA")
		return
	if energy < 1:
		show_msg_box("ZA MAŁO ENERGII !!!")
		return
	var check:bool = _client.send("0:GetCard:")

		
func _draw5Cards(): #jeszcze nieużywana
	if energy>2:
		var playerHandCopy = playerHand.duplicate(true)
		for card in playerHandCopy:
			discard(card)
		for i in range(5):
			addCardToHand(drawPile.pop_front())
			print("draw pile: ")
			print(drawPile)
		energy-=3
		$Energy.energyChange(energy)
#		emit_signal("enrgyChange",energy)
	else:
		print("Za mało Energi!!") # Some pop-up here

func discard(id:int): #jeszcze nieużywana
	var position = playerHand.find(id)
	discardPile.push_front(playerHand.pop_at(position))
	print("Do usunięcia "+"PlayerHand/Card"+str(id))
	get_node("PlayerHand/Card"+str(id)).queue_free()
	
func addCardToHand(id:int): # add new card to Hand and display it
	playerHand.append(id)
	var node = playableCard.instantiate()
	node.set_name("Card"+str(id))
	node.set_icon(id)
	if id<100:
		node.connect("play", _play_card)
	else:
		node.connect("playModificator",_play_modificator)
	%PlayerHand.add_child(node)
	print(id)

func addMonster(id:int): # add new monster
	monsters.append(id)
	var node = monsterCard.instantiate()
	node.set_name("Monster"+str(id))
	node.set_icon(id)
	$Monsters.add_child(node)
	node = monsterCardViewBtn.instantiate()
	node.set_name("Monster"+str(id))
	node.set_icon(id)
	$CenterContainer/MonsterView/HBoxContainer.add_child(node)
	
func changeHeroClass(name:String,val:int): # add new Hero's class
	clasCounter[name] += val
	get_node("EnergyBall/"+name+"/Label").text="1"
	checkHeroClass()
	print(clasCounter)
	
func checkHeroClass():
	for name in clasCounter:
		if clasCounter[name]>0: # tu może zmienić warunek aby sprawdzał jeszcze czy nie jest już zrobiona ta textura
			var texture = load("res://Images/Klasy/"+name+".png")
			get_node("EnergyBall/"+name).texture = texture
			get_node("EnergyBall/"+name+"/Label").text= str(clasCounter[name])
		else:
			var texture = load("res://Images/Klasy/"+name+"0.png")
			get_node("EnergyBall/"+name).texture = texture

func _play_card(idCard:int,card:Node):
	if !yourTurn:
		show_msg_box("NIE TWOJA TURA")
		return
	if energy < 1:
		show_msg_box("ZA MAŁO ENERGII !!!")
		return
	if can_play_modificator:
		show_msg_box("NAJPIERW WALKA !!!")
		return
	var check:bool = _client.send(str(idCard)+":PlayHero:")
	if check:
		await signal_ok
		var node = heroCardView.instantiate()
		node.set_name("Card"+str(idCard))
		node.set_icon(idCard)
		energy -= 1
		$EnergyBall.changeEnergy(energy)
		%PlayedHeroes.add_child(node)
		%PlayerHand.remove_child(card)
		var x=0;
		if idCard%8==0:
			x=idCard/8-1
		else:
			x = idCard/8
		clasCounter[clas[x]] += 1
		checkHeroClass()
	print("After Play Card")
	
func _play_modificator(idCard:int,card:Node):
	if !can_play_modificator:
		show_msg_box("NIE MOŻESZ TERAZ ZAGRAĆ MODYFIKATORA")
		return
	var check:bool = _client.send(str(idCard)+":PlayMod:")
	if check:
		await signal_ok
		%PlayerHand.remove_child(card)

func _slay_monster(idMonster:int):
	if !yourTurn:
		show_msg_box("NIE TWOJA TURA")
		return
	if idMonster==0:
		show_msg_box("NIEPOPRAWNY WYBÓR")
		return
	if energy < 2:
		show_msg_box("ZA MAŁO ENERGII !!!")
		return
	var check:bool = _client.send(str(idMonster)+":SlayMonster:")

func show_msg_box(msg:String):
	$EnergyMsg/Label.text = msg
	$EnergyMsg.show()
	%DisplayMsg.start()
	await %DisplayMsg.timeout
	$EnergyMsg.hide()

func _on_exit_btn_pressed():
	get_tree().quit()


func _on_accept_dice_val_pressed():
	var check:bool = _client.send("0:AcceptDiceVal:")
	await signal_ok


func _on_connect_pressed():
	# connect to Server
	_client.connect_to_host(HOST, PORT)
	await _client.connected
	var check:bool = _client.send($ConnectPanel/VBoxContainer/LineEdit.text)
	if check:
		$ConnectPanel/VBoxContainer/Label2.text = "Czekanie na drugiego gracza..."
		$ConnectPanel/VBoxContainer/Label2.show()

func waitForStartResources():
	$ConnectPanel/VBoxContainer/Label2.text = "Ładowanie..."
	await signal_ok
	$ConnectPanel.queue_free()

func turn(whose:int):
	if whose==1:
		yourTurn = true
		energy = 3
		$EnergyBall.changeEnergy(energy)
		$WhoseTurn.text = "TWOJA TURA"
	else:
		yourTurn = false
		energy = 0
		$EnergyBall.changeEnergy("")
		$WhoseTurn.text = "TURA PRZECIWNIKA"


func _on_end_turn_btn_pressed():
	if can_play_modificator:
		show_msg_box("NAJPIERW WALKA !!!")
		return
	if yourTurn:
		var check:bool = _client.send("0:EndTurn:")
		if check:
			turn(0)
	else:
		show_msg_box("NIE TWOJA TURA")
	
