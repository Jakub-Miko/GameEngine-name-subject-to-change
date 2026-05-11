.PHONY: build pack pack-windows clean pack-code-github pack-code-whole

build:
	mkdir -p out
	cd out && cmake .. -DRENDER_API=Vulkan -DEDITOR=On -DCMAKE_BUILD_TYPE=Release
	cd out && cmake --build . --target GameEngine

clean: 
	rm -rf ./out

pack:
	zip -r GameEngine.zip ./assets ./config*.json ./out/GameEngine ./out/*.so ./README.md ./THIRD_PARTY_NOTICES.md ./LICENSES/*

pack-windows:
	zip -r GameEngine.zip ./assets ./config*.json ./out/GameEngine.exe ./out/*.dll ./README.md ./THIRD_PARTY_NOTICES.md ./LICENSES/*

pack-code-github:
	zip -r GameEngineCode.zip . \
      -x "out/*" \
      -x "assets/necropolis/*" \
      -x "assets/sun temple/*" \
      -x "cmake*/*" \
      -x ".idea/*" \
      -x ".git*" \
      -x "temp/*" \
      -x ".git/*"

pack-code-whole:
	zip -r GameEngineCode.zip . \
      -x "out/*" \
      -x "cmake*/*" \
      -x ".idea/*" \
      -x ".git*" \
      -x "temp/*" \
      -x ".git/*"