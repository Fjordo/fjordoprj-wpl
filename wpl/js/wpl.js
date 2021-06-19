function parseData(misurazioni) {
    console.log("Calcolo Distanze");

    //  console.log(misurazioni);

    var measurements = [];
    misurazioni.forEach(element => {
        var measure = [];

        measure.push(new Date(element.data_misurazione).getTime());
        measure.push(parseInt(element.distanza));
        measurements.push(measure);
        console.log(measure);
    });

    return measurements;

}