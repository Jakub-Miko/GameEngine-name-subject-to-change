@Entity 

{
  "Properties": [

  ],

  "Children": [
    "TestEntity.lua","TestEntity2.lua"
  ]

}


@Entity:Construction_Script 


function OnConstruct()

jit.on()

end
