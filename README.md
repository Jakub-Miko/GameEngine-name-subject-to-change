# Spustenie a použitie implementácie

>## Scény a assety
> Na githube sa z licenčńích dôvodov nachádza len scéna Sponza, 
ostatné scény použité v bakalárskej práci sú obsiahnuté len v elektronickom odovzdaní. 

>## Licencia
>Kód tohto programu je licensovaný pod MIT licenciou (súbor [LICENSE](LICENSE)). Atribúcia externých zdrojov a
knižníc spolu s ich licenciami sú opísané v súbore [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Spustenie implementácie

Aplikáciu je možné spustiť pomocou spustiteľného súboru: `GameEngine`

Engine pri štarte automaticky vyhľadáva konfiguračný súbor
`config.json`. Spustiteľný súbor preto musí byť spustený z pracovného
adresára, ktorého nadradený adresár obsahuje tento súbor.

Typická štruktúra adresárov môže vyzerať nasledovne:

```
Project/
  config.json
  assets/
  out/
    GameEngine.exe
    *.dll|so
    ...
```

V tomto prípade musí byť aplikácia spustená z adresára `out`.

Ak je potrebné spustiť engine z iného pracovného adresára alebo použiť inú konfiguráciu, je možné explicitne zadať cestu ku konfiguračnému súboru pomocou argumentu:

```
config=<path_to_config.json>
```

Príklad:

```
GameEngine.exe config=D:/Projects/MyProject/config.json
```

alebo na Linuxe:

```
./GameEngine config=/home/user/project/config.json
```

## Kompilácia zo zdrojového kódu

Zdrojový kód projektu je dostupný v odovzdaní alebo na GitHub repozitári:

https://github.com/Jakub-Miko/GameEngine-name-subject-to-change/

Použitá vetva: `cleaned_up_clustered_shading`

Po klonovaní repozitára je potrebné inicializovať všetky submoduly:

```bash
git submodule update --init --recursive
```

### Kompilácia na systéme Windows

Projekt využíva build systém CMake a môže byť otvorený priamo vo Visual Studio alebo vygenerovaný pomocou CMake generátora.

Pred kompiláciou je potrebné nastaviť nasledovné CMake premenné:

- `RENDER_API=Vulkan`
- `EDITOR=On`
- `CMAKE_BUILD_TYPE=Release`

Príklad generovania projektu:

```bash
cmake -B out -DRENDER_API=Vulkan -DEDITOR=On -DCMAKE_BUILD_TYPE=Release
```

Následne je možné projekt otvoriť vo Visual Studio alebo skompilovať cez:

```bash
cmake --build out --target GameEngine
```

Výsledný spustiteľný súbor spolu so všetkými potrebnými `.dll` knižnicami musí byť umiestnený v podadresári adresára obsahujúceho `config.json`.

### Kompilácia na systéme Linux

Na systéme Linux sa požadované závislosti môžu líšiť podľa distribúcie. Predpokladá sa funkčné Vulkan prostredie a správne nainštalované ovládače GPU.

1. V koreňovom adresári projektu vytvorte build adresár:
```bash
mkdir out
```

2. Prejdite do build adresára:
```bash
cd out
```

3. Vygenerujte projekt pomocou CMake:
```bash
cmake .. -DRENDER_API=Vulkan -DEDITOR=On -DCMAKE_BUILD_TYPE=Release
```

4. Skompilujte projekt:
```bash
cmake --build . --target GameEngine
```

5. Spustite engine:
```bash
./GameEngine
```

## Použitie implementácie

Po spustení aplikácie sa načíta predvolená scéna spolu s editorovým rozhraním.

V pravej časti sa nachádza panel *Scene Graph*, ktorý obsahuje všetky entity aktuálnej scény. Po označení entity sa v ľavej časti rozhrania v paneli *Properties* zobrazia všetky jej komponenty a ich parametre.

### Animácia kamery

Komponent **Animation** obsahuje tlačidlo `Play`, ktoré spustí animovaný prelet scénou.

K dispozícii sú aj ďalšie parametre:

- rýchlosť prehrávania animácie
- opakovanie animácie

### Primárna kamera

Komponent **Camera** obsahuje voľbu `Primary`, ktorá určuje kameru použitú pri renderovaní scény.

Pri scénach obsahujúcich viacero kamier je možné dynamicky prepínať aktívnu kameru nastavením tejto voľby.

### Nastavenie renderera

Nastavenia renderovacej implementácie sú dostupné v menu:

`Settings > Viewport Settings`

Toto okno umožňuje:

- výber renderovacej implementácie
- konfiguráciu parametrov cullingu
- nastavenie debug režimov

Po zmene niektorých nastavení je potrebné potvrdiť konfiguráciu tlačidlom: `Update Culling Pipeline`

**⚠️ Pozor:** Pri zmene renderovacej implementácie za behu môže dôjsť k nestabilitám, v prípade takýchto nestabilít je lepšie použiť položku v konfiguračnom súbore s kľúčom `UseRenderer`, napríklad:

```json
"UseRenderer": "DeferredLegacy"
```

Ako hodnotu je možné použiť:

- **DeferredLegacy** - Pre originálnu deferred variantu
- **DeferredClustered** - Pre clustered deferred variantu
- **ForwardClustered** - Pre clustered forward variantu

V hlavnom adresári sú aj pred pripravené konfiguračné súbory s týmito možnosťami pre scénu *Sponza*.

### Načítavanie scén

Scéna je reprezentovaná priečinkom obsahujúcim súbor s príponou `.scene`. Na načítanie scény je najprv potrebné nájsť tento súbor v okne *File Explorer*.

Po kliknutí na požadovaný `.scene` súbor sa tento súbor označí ako aktuálne zvolený. Následne je možné v hornom menu otvoriť:

`Load > Load Scene`

Po otvorení okna pre načítanie scény je potrebné stlačiť tlačidlo: `Set Selected`

Týmto sa automaticky vyplní cesta k aktuálne zvolenému `.scene` súboru.

Scéna sa následne načíta stlačením tlačidla: `Load`

### Spustenie scén s vlastnou konfiguráciou

Pribalené ukážkové scény obsahujú aj vlastné konfiguračné súbory `config.json`, ktoré definujú potrebné nastavenia projektu, cesty k assetom a konfiguráciu renderera.

Pri spustení enginu s takouto scénou je preto potrebné explicitne zadať cestu ku konfiguračnému súboru pomocou argumentu:

`config=<cesta_ku_config.json>`

Príklad spustenia na systéme Windows:

```
GameEngine.exe config=D:/Scenes/Sponza/config.json
```

Príklad spustenia na systéme Linux:

```
./GameEngine config=/home/user/scenes/Sponza/config.json
```

---

# Štruktúra zdrojového kódu

Táto kapitola popisuje časti zdrojového kódu enginu relevantné pre implementáciu clustered shading renderera a jeho integráciu do existujúceho vykresľovacieho systému.

## Renderer

Hlavná časť implementácie renderera sa nachádza v adresári:

`Renderer/Renderer3D`

Táto časť projektu obsahuje implementáciu jednotlivých fáz vykreslenia, renderovacích techník a spoločnej infraštruktúry renderera.

### Clustered renderer

Adresár:

`Renderer/Renderer3D/ClusteredRenderer`

obsahuje implementáciu fáz vykreslenia špecifických pre clustered deferred a clustered forward renderer.

Najdôležitejšie triedy:

- **ClusteredLightCullingPass**
    - generovanie zoznamov svetiel pre klastre

- **ClusteredLightingPass**
    - osvetľovací prechod clustered deferred varianty

- **ClusteredForwardPass**
    - dopredný prechod clustered forward varianty

- **ActiveClusterFilterPass**
    - filtrovanie aktívnych klastrov z hĺbkového bufferu

- **DebugOverlayPass**
    - debug vizualizácia klastrov a hustoty osvetlenia

- **BindlessShadowMappingPass**
    - generovanie shadow máp pomocou bindless prístupu ku zdrojom

### Deferred renderer

Adresár:

`Renderer/Renderer3D/DeferredRenderer`

obsahuje implementáciu pôvodného deferred renderera, ktorý slúži ako základ pre clustered deferred variantu a zároveň ako referenčná implementácia pri porovnávaní výkonu.

Najdôležitejšie triedy:

- **GenerateGBufferPass**
    - vytváranie G-bufferu

- **DeferredGeometryPass**
    - geometrický prechod zapisujúci dáta do G-bufferu

- **DeferredLightingPass**
    - osvetľovací prechod pôvodnej deferred varianty

### Spoločné fázy vykreslenia

Adresár:

`Renderer/Renderer3D/CommonRenderPasses`

obsahuje fázy vykreslenia zdieľané medzi jednotlivými renderovacími technikami.

Najdôležitejšie triedy:

- **DepthPrepass**
    - generovanie hĺbkového bufferu pred hlavným vykreslením

- **PostProcessingPass**
    - tone mapping a post-processing výsledného obrazu

- **SkyboxPass**
    - vykreslenie skyboxu

- **RenderSubmissionPass**
    - extrakcia dát zo scény a CPU frustum culling

### Renderovacie techniky

Definície jednotlivých renderovacích techník sa nachádzajú v súboroch:

- **DeferredRenderingPipeline**
    - pôvodný deferred renderer

- **DeferredClusteredRendererPipeline**
    - clustered deferred renderer

- **ForwardClusteredRendererPipeline**
    - clustered forward renderer

### Spoločná infraštruktúra renderera

Súbory:

- `RenderPipeline`
- `RenderPass`
- `RenderPassBuilder`
- `Renderer3D`

obsahujú spoločnú infraštruktúru renderera.

Táto časť projektu zabezpečuje:

- správu jednotlivých fáz vykreslenia
- vytváranie závislostí medzi fázami
- zostavenie renderovacích techník
- správu renderovacích zdrojov
- vykonávanie renderovacích techník

## Shader programy

Shader programy použité rendererom sa nachádzajú v adresári:

`assets/Vulkan/shaders`

Táto časť projektu obsahuje vertex, fragment a compute shadery využívané jednotlivými fázami vykresľovania.

### Geometrický prechod

Súbory:

- `GeometryPassShader.glsl`
- `GeometryPassShaderEditor.glsl`
- `SkeletalGeometryPassShader.glsl`
- `SkeletalGeometryPassShaderEditor.glsl`

implementujú geometrický prechod deferred rendererov a zápis dát do G-bufferu.

Skeletal varianty rozširujú implementáciu o podporu kostrových animácií.

### Depth prepass

Súbory:

- `DepthPrepassShader.glsl`
- `SkeletalDepthPrepassShader.glsl`

slúžia na generovanie hĺbkového buffera pred hlavným vykreslením.

### Osvetľovací prechod

Súbory:

- `LightingPassShader.glsl`
- `LightingPassShaderShadowedDirectional.glsl`
- `LightingPassShaderShadowedPoint.glsl`
- `LightingPassShaderSkylight.glsl`

implementujú osvetľovací prechod pôvodného deferred renderera.

### Shadow mapping

Súbory:

- `ShadowMappingShaderDirectional.glsl`
- `ShadowMappingShaderDirectionalSkeletal.glsl`
- `ShadowMappingShaderPoint.glsl`
- `ShadowMappingShaderPointSkeletal.glsl`

implementujú generovanie shadow máp pre smerové a bodové svetlá.

### Post-processing

Súbory:

- `PostProcessingShader.glsl`
- `PostProcessingShaderWithOverlay.glsl`

implementujú tone mapping a kompozíciu debug výstupu.

### Clustered renderer shadery

Adresár:

`assets/Vulkan/shaders/ClusteredRenderer`

obsahuje shader programy špecifické pre clustered deferred a clustered forward renderer.

Najdôležitejšie shaderové programy:

- **ClusteredLightCullingShader.glsl**
    - compute shader generujúci zoznamy svetiel pre klastre

- **ClusteredLightingPassShader.glsl**
    - fragment shader osvetľovacieho prechodu clustered deferred varianty

- **ClusteredLightingPassShaderCompute.glsl**
    - compute shader implementujúci clustered deferred osvetlenie

- **ClusteredForwardPassShader.glsl**
- **ClusteredForwardPassShaderEditor.glsl**
    - shadery dopredného prechodu clustered forward varianty

- **ActiveClusterFilterShader.glsl**
    - filtrovanie aktívnych klastrov z hĺbkového bufferu

- **DebugOverlay.glsl**
    - debug vizualizácia klastrov a hustoty osvetlenia

### Pomocné shader knižnice

Adresár:

`assets/Vulkan/shaders/utils`

obsahuje pomocné shader utility využívané naprieč rendererom.

Najdôležitejšie súbory:

- **PBR.glsl**
    - implementácia PBR shading modelu

- **NormalPacking.glsl**
    - kódovanie a dekódovanie normálových vektorov

- **PointAttenuationFalloff.glsl**
    - attenuation funkcia bodových svetiel
