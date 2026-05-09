.PHONY: build pack pack-windows clean

build:
	mkdir -p out
	cd out && cmake .. -DRENDER_API=Vulkan -DEDITOR=On -DCMAKE_BUILD_TYPE=Release
	cd out && cmake --build . --target GameEngine

clean: 
	rm -rf ./out

pack:
	zip -r GameEngine.zip ./assets ./config*.json ./out/GameEngine ./out/*.so ./README.md

pack-windows:
	zip -r GameEngine.zip ./assets ./config*.json ./out/GameEngine.exe ./out/*.dll ./README.md
