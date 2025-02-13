#include "D3DHooks.hpp"

/// <summary>
/// IDirect3DDevice9::DrawPrimitive Middleware. Mainly used for Note Tails
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="PrimType"> - Member of the D3DPRIMITIVETYPE enumerated type, describing the type of primitive to render.</param>
/// <param name="StartIndex"> - Index of the first vertex to load.</param>
/// <param name="PrimCount"> - Number of primitives to render.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. If the method fails, the return value can be D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_DP(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimType, UINT StartIndex, UINT PrimCount) { // Mainly used for Note Tails
	if (pDevice->GetStreamSource(0, &Stream_Data, &Offset, &Stride) == D3D_OK)
		Stream_Data->Release();

	// Extended Range / Custom Colors
	if (AttemptedERInThisSong && UseEROrColorsInThisSong && NOTE_TAILS) {
		MemHelpers::ToggleCB(UseERExclusivelyInThisSong);

		switch (Settings::GetModSetting("SeparateNoteColors")) {
			case 0: // Use same color scheme on notes as we do on strings
				pDevice->SetTexture(1, customStringColorTexture);
				break;
			case 1: // Default Colors, so don't do anything.
				break;
			case 2: // Use Custom Note Color Scheme
				pDevice->SetTexture(1, customNoteColorTexture);
				break;
			default:
				break;
		}
	}

	if (Settings::IsTwitchSettingEnabled("RemoveNotes") && NOTE_TAILS)
		return REMOVE_TEXTURE;

	if (Settings::IsTwitchSettingEnabled("TransparentNotes") && NOTE_TAILS)
		pDevice->SetTexture(1, nonexistentTexture);

	// Solid Notes Twitch Reward
	if (Settings::IsTwitchSettingEnabled("SolidNotes") && NOTE_TAILS) {
		if (Settings::ReturnSettingValue("SolidNoteColor") == "random")
			pDevice->SetTexture(1, randomTextures[currentRandomTexture]);
		else
			pDevice->SetTexture(1, twitchUserDefinedTexture);
	}
	
	// Rainbow Notes
	if (ERMode::RainbowNotesEnabled && ERMode::customNoteColorH > 0 && NOTE_TAILS) // Rainbow Notes
		pDevice->SetTexture(1, rainbowTextures[ERMode::customNoteColorH]);

	return oDrawPrimitive(pDevice, PrimType, StartIndex, PrimCount);
}

/// <summary>
/// IDirect3DDevice9::SetVertexDeclaration Middleware.
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="pdecl"> - Pointer to an IDirect3DVertexDeclaration9 object, which contains the vertex declaration.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. The return value can be D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_SetVertexDeclaration(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pdecl) {
	if (pdecl != NULL)
		pdecl->GetDeclaration(decl, &NumElements);

	return oSetVertexDeclaration(pDevice, pdecl);
}

/// <summary>
/// IDirect3DDevice9::SetVertexShaderConstantF Middleware.
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="i_StartRegister"> - Register number that will contain the first constant value.</param>
/// <param name="pConstantData"> - Pointer to an array of constants.</param>
/// <param name="Vector4fCount"> - Number of four float vectors in the array of constants.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. If the method fails, the return value can be D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_SetVertexShaderConstantF(LPDIRECT3DDEVICE9 pDevice, UINT i_StartRegister, const float* pConstantData, UINT Vector4fCount) {
	if (pConstantData != NULL) {
		StartRegister = i_StartRegister;
		VectorCount = Vector4fCount;
	}

	return oSetVertexShaderConstantF(pDevice, i_StartRegister, pConstantData, Vector4fCount);
}

/// <summary>
/// IDirect3DDevice9::SetVertexShader Middleware.
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="veShader"> - Vertex shader interface.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. If the method fails, the return value can be D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_SetVertexShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9* veShader) {
	if (veShader != NULL) {
		vShader = veShader;
		vShader->GetFunction(NULL, &vSize);
	}

	//// Thicc fretboard note indicators.
	//if (vSize == 480 && MemHelpers::IsInStringArray(currentMenu, NULL, songModes))
	//	return D3D_OK;
	
	return oSetVertexShader(pDevice, veShader);
}

/// <summary>
/// IDirect3DDevice9::SetPixelShader Middleware.
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="piShader"> - Pixel shader interface.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. If the method fails, the return value can be D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_SetPixelShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9* piShader) {
	if (piShader != NULL) {
		pShader = piShader;
		pShader->GetFunction(NULL, &pSize);
	}

	return oSetPixelShader(pDevice, piShader);
}

/// <summary>
/// IDirect3DDevice9::SetStreamSource Middleware.
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="StreamNumber"> - Specifies the data stream, in the range from 0 to the maximum number of streams -1.</param>
/// <param name="pStreamData"> - Pointer to an IDirect3DVertexBuffer9 interface, representing the vertex buffer to bind to the specified data stream.</param>
/// <param name="OffsetInBytes"> - Offset from the beginning of the stream to the beginning of the vertex data, in bytes.</param>
/// <param name="i_Stride"> - Stride of the component, in bytes.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. If the method fails, the return value can be D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_SetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT i_Stride) {
	D3DVERTEXBUFFER_DESC desc;

	// Remove Line Markers
	if (i_Stride == 32 && NumElements == 8 && VectorCount == 4 && decl->Type == 2) { 
		pStreamData->GetDesc(&desc);
		vertexBufferSize = desc.Size;
	}

	return oSetStreamSource(pDevice, StreamNumber, pStreamData, OffsetInBytes, i_Stride);
}

/// <summary>
/// IDirect3DDevice9::Reset Middleware. Required so Alt+Tab won't break the game (ImGUI & UI Text).
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="pPresentationParameters"> - Pointer to a D3DPRESENT_PARAMETERS structure, describing the new presentation parameters. This value cannot be NULL.</param>
/// <returns>Possible return values include: D3D_OK, D3DERR_DEVICELOST, D3DERR_DEVICEREMOVED, D3DERR_DRIVERINTERNALERROR, or D3DERR_OUTOFVIDEOMEMORY.</returns>
HRESULT APIENTRY D3DHooks::Hook_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	// Lost Device
	ImGui_ImplDX9_InvalidateDeviceObjects();

	if (MemHelpers::DX9FontEncapsulation)
		MemHelpers::DX9FontEncapsulation->OnLostDevice();

	// Reset Device
	HRESULT ResetReturn = oReset(pDevice, pPresentationParameters);

	ImGui_ImplDX9_CreateDeviceObjects();

	if (MemHelpers::DX9FontEncapsulation)
		MemHelpers::DX9FontEncapsulation->OnResetDevice();

	return ResetReturn;
}

/// <summary>
/// IDirect3DDevice9::DrawIndexedPrimitive Middleware. This is where most of our texture modifying mods are located.
/// </summary>
/// <param name="pDevice"> - Device Pointer</param>
/// <param name="PrimType"> - Member of the D3DPRIMITIVETYPE enumerated type, describing the type of primitive to render.</param>
/// <param name="BaseVertexIndex"> - Offset from the start of the vertex buffer to the first vertex.</param>
/// <param name="MinVertexIndex"> - Minimum vertex index for vertices used during this call. This is a zero based index relative to BaseVertexIndex.</param>
/// <param name="NumVertices"> - Number of vertices used during this call.</param>
/// <param name="StartIndex"> - Index of the first index to use when accesssing the vertex buffer.</param>
/// <param name="PrimCount"> - Number of primitives to render.</param>
/// <returns>If the method succeeds, the return value is D3D_OK. If the method fails, the return value can be the following: D3DERR_INVALIDCALL.</returns>
HRESULT APIENTRY D3DHooks::Hook_DIP(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT StartIndex, UINT PrimCount) { // Draw things on screen
	static bool calculatedCRC = false, calculatedHeadstocks = false, calculatedSkyline = false;

	if (pDevice->GetStreamSource(0, &Stream_Data, &Offset, &Stride) == D3D_OK)
		Stream_Data->Release();

	// This could potentially lead to game locking up (because DIP is called multiple times per frame) if that value is not filled, but generally it should work 
	if (Settings::ReturnSettingValue("ExtendedRangeEnabled").length() < 2) { // Due to some weird reasons, sometimes settings decide to go missing - this may solve the problem
		Settings::UpdateSettings();
		D3D::GenerateTextures(pDevice, D3D::Strings);
		D3D::GenerateTextures(pDevice, D3D::Notes);
		std::cout << "Reloaded settings" << std::endl;
	}

	if (setAllToNoteGradientTexture) {
		pDevice->SetTexture(currStride, gradientTextureSeven);
		return SHOW_TEXTURE;
	}

	Mesh current(Stride, PrimCount, NumVertices);
	ThiccMesh currentThicc(Stride, PrimCount, NumVertices, StartIndex, StartRegister, PrimType, decl->Type, VectorCount, NumElements);

	if (debug) {
		if (GetAsyncKeyState(VK_PRIOR) & 1 && currIdx < std::size(allMeshes) - 1)// Page up
			currIdx++;
		if (GetAsyncKeyState(VK_NEXT) & 1 && currIdx > 0) // Page down
			currIdx--;

		if (GetAsyncKeyState(VK_END) & 1) { // Toggle logging
			std::cout << "Logging is ";
			startLogging = !startLogging;
			if (!startLogging)
				std::cout << "no longer ";
			std::cout << "armed!" << std::endl;
		}

		if (GetAsyncKeyState(VK_F8) & 1) { // Save logged meshes to file
			for (const auto& mesh : allMeshes) {
				//Log(mesh.ToString().c_str());
			}
		}

		if (GetAsyncKeyState(VK_F7) & 1) { // Save only removed 
			for (const auto& mesh : removedMeshes) {
				//Log(mesh.ToString().c_str());
			}
		}

		if (GetAsyncKeyState(VK_CONTROL) & 1)
			//Log("{ %d, %d, %d, %d, %d, %d, %d, %d, %d }, ", Stride, PrimCount, NumVertices, StartIndex, StartRegister, PrimType, decl->Type, VectorCount, NumElements);

			if (startLogging) {
				if (std::find(allMeshes.begin(), allMeshes.end(), currentThicc) == allMeshes.end()) // Make sure we don't log what we'd already logged
					allMeshes.push_back(currentThicc);
				if (NOTE_STEMS) // Criteria for search
					std::cout << "{ " << Stride << ", " << PrimCount << ", " << NumVertices << ", " << StartIndex << ", " << StartRegister << ", " << (UINT)PrimType << ", " << (UINT)decl->Type << ", " << VectorCount << ", " << NumElements << " },"<< std::endl; // Thicc Mesh -> Console
				//std::cout << "{ "<< Stride << ", " << PrimCount << ", " << NumVertices << " }," std::endl; // Mesh -> Console
				//Log("{ %d, %d, %d},", Stride, PrimCount, NumVertices); // Mesh -> Log File
				//Log("{ %d, %d, %d, %d, %d, %d, %d, %d, %d }, ", Stride, PrimCount, NumVertices, StartIndex, StartRegister, PrimType, decl->Type, VectorCount, NumElements); // ThiccMesh -> Log File
				//std::cout << std::hex << crc << std::endl;
			}

		if (std::size(allMeshes) > 0 && allMeshes.at(currIdx) == currentThicc) {
			currStride = Stride;
			currNumVertices = NumVertices;
			currPrimCount = PrimCount;
			currStartIndex = StartIndex;
			currStartRegister = StartRegister;
			currPrimType = PrimType;
			currDeclType = decl->Type;
			currVectorCount = VectorCount;
			currNumElements = NumElements;
			//pDevice->SetTexture(1, Yellow);
			return REMOVE_TEXTURE;
		}

		if (IsExtraRemoved(removedMeshes, currentThicc))
			return REMOVE_TEXTURE;
	}

	// Mods

	// Change Noteway Color | This NEEDS to be above Extended Range / Custom Colors or it won't work.
	if (IsToBeRemoved(noteHighway, current) && Settings::ReturnSettingValue("CustomHighwayColors") == (std::string)"on") {
		pDevice->GetTexture(1, &pBaseNotewayTexture);
		pCurrNotewayTexture = (IDirect3DTexture9*)pBaseNotewayTexture;

		if (pBaseNotewayTexture) {
			if (CRCForTexture(pCurrNotewayTexture, pDevice, crc)) {
				if (crc == crcNoteLanes && Settings::ReturnNotewayColor("CustomHighwayNumbered") != (std::string)"" && Settings::ReturnNotewayColor("CustomHighwayUnNumbered") != (std::string)"")
					pDevice->SetTexture(1, notewayTexture);
				else if (crc == crcNotewayFretNumbers && Settings::ReturnNotewayColor("CustomFretNubmers") != (std::string)"")
					pDevice->SetTexture(1, fretNumTexture);
				else if (crc == crcNotewayGutters && Settings::ReturnNotewayColor("CustomHighwayGutter") != (std::string)"")
					pDevice->SetTexture(1, gutterTexture);
			}
		}
	}

	//if (IsExtraRemoved(headstonks, currentThicc) && (GetAsyncKeyState(VK_CONTROL) & 1)) {

	//	// Save Headstock Texture to file
	//	pDevice->GetTexture(1, &pBaseTextures[1]);
	//	D3DXSaveTextureToFile(L"headstock.png", D3DXIFF_PNG, pBaseTextures[1], NULL);

	//	// Use Custom Texture
	//	pDevice->SetTexture(1, customHeadstockTexture);
	//}


	// // Custom Loft Gameplay Wall / Narnia / Portal / Venue wall
	//if (IsExtraRemoved(greenScreenWallMesh, currentThicc)) {
	//		//// Save Loft Texture To File
	//	//DumpTextureStages(pDevice, "greenscreenwall");

	//	// Use Custom Texture (File names can be found in venues/loft01.psarc/assets/generic/env/the_loft/
	//	// Files sent in currently require the name "stage#.png" where # is the number attached to the texture variable. Ex: customGreenScreenWall_Stage3 would need a file named "stage3.png"

	//	// Background tile displays as follows:
	//	// Top 512 of Background Tile are shown at the bottom of the screen, repeated 1-1/2 times.
	//	// Bottom 512 of Background Tile are shown at the top of the screen, repeated 2-1/2 times.

	//	// Example: What is shown - https://cdn.discordapp.com/attachments/711634485388771439/813523398587711488/unknown.png vs What is sent - https://cdn.discordapp.com/attachments/711634485388771439/813523420947152916/stage0.png
	//	// Example: Full wall (which you will never see for more than a second or so) - https://cdn.discordapp.com/attachments/711634485388771439/813524110390460436/unknown.png

	//	pDevice->SetTexture(0, customGreenScreenWall_Stage0); // Background Tile | loft_concrete_wall_b.dds | 1024x1024 | Can be modified. Used for the background.
	//	//pDevice->SetTexture(1, customGreenScreenWall_Stage1); // Noise | noise03.dds | 256x256 | Doesn't have any effect
	//	//pDevice->SetTexture(2, customGreenScreenWall_Stage2); // Caustic (Indirect) | caustic_indirect01.dds | 256x256 | Doesn't have any effect
	//	//pDevice->SetTexture(3, customGreenScreenWall_Stage3); // Narnia / Venue Fade In Mask | fade_shape.dds | 512x512 | Can be modified. If you use a single colored square, you can make an almost "movie like" flashback.
	//	//pDevice->SetTexture(4, customGreenScreenWall_Stage4); // White square (Unknown) | 1024x1024 | Doesn't have any effect
	//	//pDevice->SetTexture(5, customGreenScreenWall_Stage5); // Pipes and wall trim | portal_wall_ao.dds | 1024x1024 | Can be modified.
	//	//pDevice->SetTexture(6, customGreenScreenWall_Stage6); // N Mask of Background tile | loft_concrete_wall_b_n.dds | 1024x1024 | Don't modify
	//}

	// Rainbow Notes | This part NEEDS to be above Extended Range / Custom Colors or it won't work.
	if (ERMode::RainbowNotesEnabled && ERMode::customNoteColorH > 0) { 

		if (ERMode::customNoteColorH > 179)
			ERMode::customNoteColorH -= 180;

		RainbowNotes = true;
		if (NOTE_STEMS || OPEN_NOTE_ACCENTS) {
			pDevice->GetTexture(1, &pBaseRainbowTexture);
			pCurrRainbowTexture = (IDirect3DTexture9*)pBaseRainbowTexture;

			if (!pBaseRainbowTexture)
				return SHOW_TEXTURE;

			if (CRCForTexture(pCurrRainbowTexture, pDevice, crc)) {
				if (crc == crcStemsAccents || crc == crcBendSlideIndicators)
					pDevice->SetTexture(1, rainbowTextures[ERMode::customNoteColorH]);
			}
		}

		if (PrideMode && NOTE_TAILS) // As of right now, this requires rainbow strings to be toggled on
			pDevice->SetTexture(1, rainbowTextures[ERMode::customNoteColorH]);
	}
		
	//if (Settings::ReturnSettingValue("DiscoModeEnabled") == "on") {
	//	 //Need Lovro's Help With This :(
	//	if (DiscoModeInitialSetting.find(pDevice) == DiscoModeInitialSetting.end()) { // We haven't saved this pDevice's initial values yet
	//		DWORD initialAlphaValue = (DWORD)pDevice, initialSeperateValue = (DWORD)pDevice;
	//		pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)initialAlphaValue);
	//		pDevice->GetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, (DWORD*)initialSeperateValue);
	//		
	//		DiscoModeInitialSetting.insert({ pDevice, std::make_pair(initialAlphaValue, initialSeperateValue) });
	//	}
	//	else { // We've seen this pDevice value before.
	//		if (DiscoModeEnabled) { // Key was pressed to have Disco Mode on
	//			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); // Make AMPS Semi-Transparent <- Is the one that makes things glitchy.
	//			pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE); // Sticky Colors
	//		}

	//		else { // Disco mode was turned off, we need to revert the settings so there is no trace of disco mode.
	//			for (auto pDeviceList : DiscoModeInitialSetting) {
	//				pDeviceList.first->SetRenderState(D3DRS_ALPHABLENDENABLE, *(DWORD*)pDeviceList.second.first); // Needs to have *(DWORD*) since it only sets DWORD not DWORD*
	//				pDeviceList.first->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, *(DWORD*)pDeviceList.second.second); // Needs to have *(DWORD*) since it only sets DWORD not DWORD*
	//			}
	//		}
	//	}
	//}

	/*if (IsExtraRemoved(lyrics, currentThicc)) { // Move Lyrics to different file. Current State: CRC never updates, BUT does pass CRCForTexture().
		pDevice->GetTexture(0, &pBaseTexture);

		D3DXSaveTextureToFile(L"lyrics_temp.png", D3DXIFF_PNG, pBaseTexture, NULL);

		D3DXCreateTextureFromFile(pDevice, L"lyrics_temp.png", &pCurrTexture);

		if (CRCForTexture(pCurrTexture, pDevice, crc)) {
			if (crc != crcLyrics) {
				D3DXSaveTextureToFile(L"lyrics.png", D3DXIFF_PNG, pBaseTexture, NULL);
				crcLyrics = crc;
				std::cout << "new lyric posted to lyrics.png" << std::endl;
			}
			std::cout << std::hex << crcLyrics << " = " << std::hex << crc << std::endl;
		}
	}*/

	// Extended Range / Custom Colors (includes separate note colors)
	if (AttemptedERInThisSong && UseEROrColorsInThisSong) {
		MemHelpers::ToggleCB(UseERExclusivelyInThisSong);

		// Settings::GetModSetting("SeparateNoteColors") == 1 -> Default Colors, so don't do anything.

		if (Settings::GetModSetting("SeparateNoteColorsMode") == 0 || (Settings::ReturnSettingValue("SeparateNoteColors") == "on" && Settings::GetModSetting("SeparateNoteColorsMode") == 2)) { // Use same color scheme on notes as we do on strings (0) || Use Custom Note Color Scheme (2)

			LPDIRECT3DTEXTURE9 textureToUseOnNotes = customStringColorTexture; // Color notes like string colors

			if (Settings::GetModSetting("SeparateNoteColorsMode") == 2)
				textureToUseOnNotes = customNoteColorTexture; // Custom colored notes

			if (IsToBeRemoved(sevenstring, current) || IsExtraRemoved(noteModifiers, currentThicc))  // Change all pieces of note head's textures
				pDevice->SetTexture(1, textureToUseOnNotes);

			else if (NOTE_STEMS || OPEN_NOTE_ACCENTS) { // Colors for note stems (part below the note), bends, slides, and accents
				pDevice->GetTexture(1, &pBaseTexture);
				pCurrTexture = (IDirect3DTexture9*)pBaseTexture;

				if (!pBaseTexture)
					return SHOW_TEXTURE;

				if (CRCForTexture(pCurrTexture, pDevice, crc)) {
					//if (startLogging)
					//	Log("0x%08x", crc);

					if (crc == crcStemsAccents || crc == crcBendSlideIndicators)  // Same checksum for stems and accents, because they use the same texture. Bends and slides use the same texture.
						pDevice->SetTexture(1, textureToUseOnNotes);
				}

				return SHOW_TEXTURE;
			}
		}
	}

	// Twitch Settings
	if (Settings::IsTwitchSettingEnabled("RemoveNotes"))
		if (IsToBeRemoved(sevenstring, current) || IsExtraRemoved(noteModifiers, currentThicc))
			return REMOVE_TEXTURE;
		else if (NOTE_STEMS || OPEN_NOTE_ACCENTS) { // Colors for note stems (part below the note), bends, slides, and accents
			pDevice->GetTexture(1, &pBaseTexture);
			pCurrTexture = (IDirect3DTexture9*)pBaseTexture;

			if (!pBaseTexture)
				return REMOVE_TEXTURE;

			if (CRCForTexture(pCurrTexture, pDevice, crc)) {
				//if (startLogging)
				//	Log("0x%08x", crc);

				if (crc == crcStemsAccents || crc == crcBendSlideIndicators)  // Same checksum for stems and accents, because they use the same texture. Bends and slides use the same texture.
					return REMOVE_TEXTURE;
			}

			return REMOVE_TEXTURE;
		}

	if (Settings::IsTwitchSettingEnabled("TransparentNotes"))
		if (IsToBeRemoved(sevenstring, current) || IsExtraRemoved(noteModifiers, currentThicc) || NOTE_STEMS || OPEN_NOTE_ACCENTS)
			pDevice->SetTexture(1, nonexistentTexture);
		else if (NOTE_STEMS || OPEN_NOTE_ACCENTS) { // Colors for note stems (part below the note), bends, slides, and accents
			pDevice->GetTexture(1, &pBaseTexture);
			pCurrTexture = (IDirect3DTexture9*)pBaseTexture;

			if (!pBaseTexture)
				return SHOW_TEXTURE;

			if (CRCForTexture(pCurrTexture, pDevice, crc)) {
				//if (startLogging)
				//	Log("0x%08x", crc);

				if (crc == crcStemsAccents || crc == crcBendSlideIndicators)  // Same checksum for stems and accents, because they use the same texture. Bends and slides use the same texture.
					pDevice->SetTexture(1, nonexistentTexture);
			}

			return SHOW_TEXTURE;
		}

	if (Settings::IsTwitchSettingEnabled("SolidNotes")) {
		if (IsToBeRemoved(sevenstring, current) || IsExtraRemoved(noteModifiers, currentThicc)) {
			if (Settings::ReturnSettingValue("SolidNoteColor") == "random") // Random Colors
				pDevice->SetTexture(1, randomTextures[currentRandomTexture]);
			else // They set the color they want in the GUI | TODO: Colors are changed on chord boxes
				pDevice->SetTexture(1, twitchUserDefinedTexture);
		}
		else if (NOTE_STEMS || OPEN_NOTE_ACCENTS) { // Colors for note stems (part below the note), bends, slides, and accents
			pDevice->GetTexture(1, &pBaseTexture);
			pCurrTexture = (IDirect3DTexture9*)pBaseTexture;

			if (!pBaseTexture)
				return SHOW_TEXTURE;

			if (CRCForTexture(pCurrTexture, pDevice, crc)) {
				//if (startLogging)
				//	Log("0x%08x", crc);

				if (crc == crcStemsAccents || crc == crcBendSlideIndicators) {  // Same checksum for stems and accents, because they use the same texture. Bends and slides use the same texture.
					if (Settings::ReturnSettingValue("SolidNoteColor") == "random") // Random Colors
						pDevice->SetTexture(1, randomTextures[currentRandomTexture]);
					else
						pDevice->SetTexture(1, twitchUserDefinedTexture);
				}
			}

			return SHOW_TEXTURE;
		}
	}

	if (Settings::IsTwitchSettingEnabled("FYourFC")) {
		uintptr_t currentNoteStreak = 0;

		if (MemHelpers::IsInStringArray(currentMenu, learnASongModes))
			currentNoteStreak = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_currentNoteStreak, Offsets::ptr_currentNoteStreakLASOffsets);
		else if (MemHelpers::IsInStringArray(currentMenu, scoreAttackModes))
			currentNoteStreak = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_currentNoteStreak, Offsets::ptr_currentNoteStreakSAOffsets);

		if (currentNoteStreak != 0)
			*(BYTE*)currentNoteStreak = 0;
	}

	if (Settings::IsTwitchSettingEnabled("DrunkMode")) {
		std::uniform_real_distribution<> keepValueWithin(-1.5, 1.5);
		*(float*)Offsets::ptr_drunkShit = (float)keepValueWithin(rng);
	}

	// Greenscreen Wall
	if ((Settings::ReturnSettingValue("GreenScreenWallEnabled") == "on" || GreenScreenWall) && IsExtraRemoved(greenScreenWallMesh, currentThicc))
		return REMOVE_TEXTURE;

	// Thicc Mesh Mods
	if (MemHelpers::IsInSong()) {
		if (Settings::ReturnSettingValue("FretlessModeEnabled") == "on" && IsExtraRemoved(fretless, currentThicc))
			return REMOVE_TEXTURE;
		if (Settings::ReturnSettingValue("RemoveInlaysEnabled") == "on" && IsExtraRemoved(inlays, currentThicc))
			return REMOVE_TEXTURE;
		if (Settings::ReturnSettingValue("RemoveLaneMarkersEnabled") == "on" && IsExtraRemoved(laneMarkers, currentThicc))
			return REMOVE_TEXTURE;
		if (RemoveLyrics && Settings::ReturnSettingValue("RemoveLyrics") == "on" && IsExtraRemoved(lyrics, currentThicc))
			return REMOVE_TEXTURE;
	}

	// Remove Headstock Artifacts
	else if (MemHelpers::IsInStringArray(currentMenu, tuningMenus) && Settings::ReturnSettingValue("RemoveHeadstockEnabled") == "on" && RemoveHeadstockInThisMenu)
	{
		if (IsExtraRemoved(tuningLetters, currentThicc)) // This is called to remove those pesky tuning letters that share the same texture values as fret numbers and chord fingerings
			return REMOVE_TEXTURE;
		if (IsExtraRemoved(tunerHighlight, currentThicc)) // This is called to remove the tuner's highlights
			return REMOVE_TEXTURE;
		if (IsExtraRemoved(leftyFix, currentThicc)) // Lefties need their own little place in life...
			return REMOVE_TEXTURE;
	}

	// Skyline Removal
	if (toggleSkyline && POSSIBLE_SKYLINE) {
		if (DrawSkylineInMenu) { // If the user is in "Song" mode for Toggle Skyline and is NOT in a song -> draw the UI
			SkylineOff = false;
			return SHOW_TEXTURE;
		}

		pDevice->GetTexture(1, &pBaseTextures[1]);
		pCurrTextures[1] = (IDirect3DTexture9*)pBaseTextures[1];

		if (pBaseTextures[1]) {  // There's only two textures in Stage 1 for meshes with Stride = 16, so we could as well skip CRC calcuation and just check if !pBaseTextures[1] and return D3D_OK directly
			if (CRCForTexture(pCurrTextures[1], pDevice, crc)) {
				if (crc == crcSkylinePurple || crc == crcSkylineOrange) { // Purple rectangles + orange line beneath them
					SkylineOff = true;
					return REMOVE_TEXTURE;
				}
			}
		}

		pDevice->GetTexture(0, &pBaseTextures[0]);
		pCurrTextures[0] = (IDirect3DTexture9*)pBaseTextures[0];

		if (pBaseTextures[0]) {
			if (CRCForTexture(pCurrTextures[0], pDevice, crc)) {
				if (crc == crcSkylineBackground || crc == crcSkylineShadow) {  // There's a few more of textures used in Stage 0, so doing the same is no-go; Shadow-ish thing in the background + backgrounds of rectangles
					SkylineOff = true;
					return REMOVE_TEXTURE;
				}
			}
		}
	}

	// Headstock Removal
	else if (Settings::ReturnSettingValue("RemoveHeadstockEnabled") == "on") {
		if (POSSIBLE_HEADSTOCKS) { // If we call GetTexture without any filtering, it causes a lockup when ALT-TAB-ing/changing fullscreen to windowed and vice versa
			if (!RemoveHeadstockInThisMenu) // This user has RemoveHeadstock only on during the song. So if we aren't in the song, we need to draw the headstock texture.
				return SHOW_TEXTURE;

			pDevice->GetTexture(1, &pBaseTextures[1]);
			pCurrTextures[1] = (IDirect3DTexture9*)pBaseTextures[1];

			if (resetHeadstockCache && IsExtraRemoved(headstockThicc, currentThicc)) {
				if (!pBaseTextures[1]) //if there's no texture for Stage 1
					return REMOVE_TEXTURE;

				if (CRCForTexture(pCurrTextures[1], pDevice, crc)) {
					if (crc == crcHeadstock0 || crc == crcHeadstock1 || crc == crcHeadstock2 || crc == crcHeadstock3 || crc == crcHeadstock4)
						AddToTextureList(headstockTexturePointers, pCurrTextures[1]);
				}

				//Log("0x%08x", crc);

				int headstockCRCLimit = 3;

				if (MemHelpers::IsInStringArray(currentMenu, multiplayerTuners))
					headstockCRCLimit = 6;

				if (headstockTexturePointers.size() == headstockCRCLimit) {
					calculatedHeadstocks = true;
					resetHeadstockCache = false;
					//std::cout << "Calculated headstock CRCs (Menu: " << currentMenu << " )" << std::endl;
				}

				return REMOVE_TEXTURE;
			}

			if (calculatedHeadstocks)
				if (std::find(std::begin(headstockTexturePointers), std::end(headstockTexturePointers), pCurrTextures[1]) != std::end(headstockTexturePointers))
					return REMOVE_TEXTURE;
		}
	}

	// Rainbow Notes || This part NEEDS to be below Extended Range / Custom Colors or it won't work.
	if (RainbowNotes) { 
		if (IsToBeRemoved(sevenstring, current) || IsExtraRemoved(noteModifiers, currentThicc)) // Note Heads
			pDevice->SetTexture(1, rainbowTextures[ERMode::customNoteColorH]);
		RainbowNotes = false;
	}

	return SHOW_TEXTURE; // KEEP THIS LINE. This translates to "Display Graphics".
}