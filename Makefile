All:
	@Make -s -C GLBlockTube
	@Make -s -C GLCubenetic
	@Make -s -C GLFlurry
	@Make -s -C GLMatrix
	@Make -s -C GLHypertorus
	@Make -s -C GLNoof
	@Make -s -C GLKnots
	@Make -s -C GLKlein
	@Make -s -C GLSchool
	@Make -s -C GLBoxed

install:
	@Make -C GLBlockTube install
	@Make -C GLCubenetic install
	@Make -C GLFlurry install
	@Make -C GLMatrix install
	@Make -C GLHypertorus install
	@Make -C GLNoof install
	@Make -C GLKnots install
	@Make -C GLKlein install
	@Make -C GLSchool install
	@Make -C GLBoxed install

clean:
	@Make -s -C GLBlockTube clean
	@Make -s -C GLCubenetic clean
	@Make -s -C GLFlurry clean
	@Make -s -C GLMatrix clean
	@Make -s -C GLHypertorus clean
	@Make -s -C GLNoof clean
	@Make -s -C GLKnots clean
	@Make -s -C GLKlein clean
	@Make -s -C GLSchool clean
	@Make -s -C GLBoxed clean

release:
	copy sys:utilities/blankers/GLBlockTube.blanker Release_folder/
	copy sys:utilities/blankers/GLBlockTube.blanker.info Release_folder/
	copy sys:utilities/blankers/GLCubenetic.blanker Release_folder/
	copy sys:utilities/blankers/GLCubenetic.blanker.info Release_folder/
	copy sys:utilities/blankers/GLFlurry.blanker Release_folder/
	copy sys:utilities/blankers/GLFlurry.blanker.info Release_folder/
	copy sys:utilities/blankers/GLMatrix.blanker Release_folder/
	copy sys:utilities/blankers/GLMatrix.blanker.info Release_folder/
	copy sys:utilities/blankers/GLHyperTorus.blanker Release_folder/
	copy sys:utilities/blankers/GLHyperTorus.blanker.info Release_folder/
	copy sys:utilities/blankers/GLNoof.blanker Release_folder/
	copy sys:utilities/blankers/GLNoof.blanker.info Release_folder/
	copy sys:utilities/blankers/GLKnots.blanker Release_folder/
	copy sys:utilities/blankers/GLKnots.blanker.info Release_folder/
	copy sys:utilities/blankers/GLKlein.blanker Release_folder/
	copy sys:utilities/blankers/GLKlein.blanker.info Release_folder/
	copy sys:utilities/blankers/GLSchool.blanker Release_folder/
	copy sys:utilities/blankers/GLSchool.blanker.info Release_folder/
	copy sys:utilities/blankers/GLBoxed.blanker Release_folder/
	copy sys:utilities/blankers/GLBoxed.blanker.info Release_folder/
	copy Readme.txt Release_folder/
	lha a GLBlankers.lha Release_folder/*




