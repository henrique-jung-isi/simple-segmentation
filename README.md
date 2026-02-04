# isisim-cmake-template

Template para projetos cmake que podem ser instalados e utilizados como alvos de `find_package`.

[CMakeLists](CMakeLists.txt#L3) deve ser alterado para conter o nome do projeto.

Escolha se o projeto vai criar um [executável](CMakeLists.txt#L7) ou uma [biblioteca](CMakeLists.txt#L12), e remova ou renomeie o outro target.

## Clonar e configurar o repositório

```{bash}
git clone https://github.com/isisim/PROJECT_NAME_HERE
cd PROJECT_NAME_HERE
mkdir build
cd build
cmake .. . -G Ninja 
# ou para release:
# cmake .. . -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build .
sudo cmake --install .
```

## Desinstalação

Utilize o target `uninstall`. Na pasta principal do projeto execute o comando:

```{bash}
sudo cmake --build build --target uninstall
```
