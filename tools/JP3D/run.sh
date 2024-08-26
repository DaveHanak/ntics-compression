#!/bin/bash

# Set these accordingly
directories=(
    "../../../../OurSet/CMB-CRC"
    "../../../../OurSet/CMB-LCA"
    "../../../../OurSet/CMB-MEL"
    "../../../../OurSet/CPTAC-LUAD"
    "../../../../OurSet/CPTAC-PDA"
    "../../../../OurSet/CPTAC-SAR"
    "../../../../OurSet/CPTAC-UCEC"
    "../../../../OurSet/QIN-BREAST"
)
origin=$(pwd)

tools=(
    "jp3d"
)

logenc="JP3D-enc.log"
logdec="JP3D-dec.log"

# Copy tools to the target directories
for dir in ${directories[@]}; do
    for tool in ${tools[@]}; do
        if [ ! -f "$dir/$tool" ]; then
            echo "$tool not present in $dir, copying."
            cp "$tool" "$dir/"
        fi
    done
done

# Encode all files
for dir in ${directories[@]}; do
    cd "$dir"
    echo "File,Time" >> "$logenc"
    find . -type f -name "*.sh" | while read -r file; do
        start=$(date +%s.%N)
        bash "$file"
        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)
        echo "$file,$elapsed" >> "$logenc"
        rm "$file"
    done
    cd "$origin"
done

# Decode all encoded files to check whether the coding was truly lossless
for dir in ${directories[@]}; do
    cd "$dir"
    echo "File,Time" >> "$logdec"
    find . -type f -name "*.jp3de" | while read -r file; do
        base=${file%.*}
        start=$(date +%s.%N)
        ./jp3d -d "$file" "$base.jp3dd"
        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)
        echo "$file,$elapsed" >> "$logdec"
    done
    cd "$origin"
done
for dir in ${directories[@]}; do
    cd "$dir"
    find . -type f -name "*.jp3dd" | while read -r file; do
        base=${file%.*}
        cmp "$file" "$base.raw"
        rm "$file"
    done
    cd "$origin"
done

# Clean up tools
for dir in ${directories[@]}; do
    for tool in ${tools[@]}; do
        if [ -f "$dir/$tool" ]; then
            rm "$dir/$tool"
        fi
    done
done