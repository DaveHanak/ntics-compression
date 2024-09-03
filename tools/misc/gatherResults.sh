directories=(
    "../../../../OurSet/CMB-CRC"
    "../../../../OurSet/CMB-LCA"
    "../../../../OurSet/CMB-MEL"
    "../../../../OurSet/CPTAC-LUAD"
    "../../../../OurSet/CPTAC-PDA"
    "../../../../OurSet/CPTAC-SAR"
    "../../../../OurSet/CPTAC-UCEC"
    "../../../../OurSet/QIN-BREAST"
    "../../../../OurSet/Bruylants"
)
origin=$(pwd)

for dir in ${directories[@]}; do
    cd "$dir"
    name=$(basename $dir)
    zip -m "$name.zip" *.csv
    cd "$origin"
done