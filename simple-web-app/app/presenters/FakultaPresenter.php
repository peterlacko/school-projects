<?php

use Nette\Application\UI\Form;
/*
 * Presenter fakulta
 */
class FakultaPresenter extends BasePresenter
{
    private $skolaRepository;
    private $fakultaRepository;
    private $oborRepository;
    private $dodRepository;
    private $predmetRepository;
    private $oblibeneRepository;
	private $akceRepository;
    private $detailFakulty;
    private $recordFakulta;
    private $recordPredmet;
    private $recordObor;

    protected function startup()
    {
		parent::startup();
		$this->skolaRepository = $this->context->skolaRepository;
		$this->fakultaRepository = $this->context->fakultaRepository;
		$this->oborRepository = $this->context->oborRepository;
		$this->dodRepository = $this->context->dodRepository;
		$this->predmetRepository = $this->context->predmetRepository;
		$this->oblibeneRepository = $this->context->oblibeneRepository;
		$this->akceRepository = $this->context->akceRepository;
    }

    public function actionDetail($id) {
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
    }

    public function actionAddobor($id) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Skola:');
	}
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
    }

    public function actionAddpredmet($id) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Skola:');
	}
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
    }

    public function actionEditobor($id_fakulty, $id_oboru) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Skola:');
	}
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id_fakulty))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
	$this->recordObor = $this->oborRepository->findBy(array('ID_oboru' => $id_oboru))->fetch();
	if (!$this->recordObor) {
	    $this->setView('oborNotFound');
	}
	$this['editOborForm']->setDefaults(array(
	    'nazev_oboru' => $this->recordObor['Nazev_oboru'],
	    'delka_studia' => $this->recordObor['Delka_studia'],
	    'forma_studia' => $this->recordObor['Forma_studia'],
	    'titul' => $this->recordObor['Titul'],
	    'zakonceni' => $this->recordObor['Zakonceni'],
	    'popis' => $this->recordObor['Popis']
	));
    }

    public function actionEditpredmet($id_fakulty, $id_predmetu) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Skola:');
	}
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id_fakulty))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
	$this->recordPredmet = $this->predmetRepository->findBy(array('ID_predmetu' => $id_predmetu))->fetch();
	if (!$this->recordPredmet) {
	    $this->setView('predmetNotFound');
	}
	$this['editPredmetForm']->setDefaults(array(
	    'zkratka_predmetu' => $this->recordPredmet['Zkratka_predmetu'],
	    'nazev_predmetu' => $this->recordPredmet['Nazev_predmetu'],
	    'kredity' => $this->recordPredmet['Kredity'],
	    'ukonceni' => $this->recordPredmet['Ukonceni'],
	    'garant' => $this->recordPredmet['Garant']
	));
    }

    public function actionEditfakulta($id_fakulty) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Skola:');
	}
	$this->recordFakulta = $this->fakultaRepository->findBy(array('ID_fakulty' => $id_fakulty))->fetch();
	if (!$this->recordFakulta) {
	    $this->setView('notFound');
	}
	$this['editFakultaForm']->setDefaults(array(
	    'nazev_fakulty' => $this->recordFakulta['Nazev_fakulty'],
	    'adresa_fakulty' => $this->recordFakulta['Adresa_fakulty'],
	    'email_fakulty' => $this->recordFakulta['Email_fakulty'],
	    'web_fakulty' => $this->recordFakulta['Web_fakulty'],
	    'dekan' => $this->recordFakulta['Dekan'],
	    'skola' => $this->recordFakulta['Nazev_skoly']
	));
    }

    public function renderDefault()
    {
		$this->template->facultiesNames = $this->fakultaRepository->findAll();
    }

    public function renderDetail() {
		$this->template->detailFakulty = $this->detailFakulty;
		$this->template->obory = $this->fakultaRepository->oborOf($this->detailFakulty);    
		$this->template->dod = $this->fakultaRepository->dodOf($this->detailFakulty);
		$this->template->predmety = $this->fakultaRepository->predmetOf($this->detailFakulty)->order('Nazev_predmetu', 'ASC');
		$this->template->akce = $this->akceRepository->findBy(array('ID_fakulty' => $this->detailFakulty->ID_fakulty));
		
		$fav = $this->oblibeneRepository->findBy(array('User_id' => $this->getUser()->getId(), 'ID_fakulty' => $this->detailFakulty->ID_fakulty))->fetch();
		if ($fav != false)
			$this->template->favId = $fav->ID_oblibene;
		else
			$this->template->favId = 0;
    }

    public function renderAddpredmet()
    {
	$this->template->detailFakulty = $this->detailFakulty;
    }

    public function renderAddobor()
    {
	$this->template->detailFakulty = $this->detailFakulty;
    }

    public function renderEditpredmet()
    {
	$this->template->detailFakulty = $this->detailFakulty;
    }

    public function renderEditobor()
    {
	$this->template->detailFakulty = $this->detailFakulty;
    }


    /*
     * pridani oboru
     */
    protected function createComponentAddOborForm() 
    {
	$form = new Form;
	$form->addText('nazev_oboru', 'Název oboru:')
	    ->setRequired('Zadejte prosím název oboru');
	$form->addText('delka_studia', 'Délka studia:')
		->setRequired('Zadejte prosím déku studia')
	    ->addCondition($form::FILLED)
	    ->addRule($form::INTEGER, 'Délka studia musí být celé kladné číslo');
	$form->addText('forma_studia', 'Forma studia:');
	$form->addText('titul', 'Titul:');
	$form->addText('zakonceni', 'Zakonceni:');
	$form->addTextArea('popis', 'Popis:')
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 300 znaku', 300);
	$form->addSubmit('addObor', 'Přidat')
	    ->onClick[] = $this->addOborFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'addOborFormCancel');
	return $form;
    }

    public function addOborFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values = $btn->form->getValues();
	if ($this->oborRepository->isOborInFakulta($values->nazev_oboru, $this->detailFakulty['ID_fakulty'])) {
	    $btn->form->addError('Obor se zadaným názvem na tyhle škole již existuje');
	    return;
	}
	try {
	    $this->oborRepository->addObor($values, $this->detailFakulty['ID_fakulty']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	}
    }

    public function addOborFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
    }

    /*
     * editace oboru
     */
    protected function createComponentEditOborForm() 
    {
	$form = new Form;
	$form->addText('nazev_oboru', 'Název oboru:')
	    ->setRequired('Zadejte prosím název oboru');
	$form->addText('delka_studia', 'Délka studia:')
	    ->addCondition($form::FILLED)
	    ->addRule($form::INTEGER, 'Délka studia musí být celé kladné číslo');
	$form->addText('forma_studia', 'Forma studia:');
	$form->addText('titul', 'Titul:');
	$form->addText('zakonceni', 'Zakonceni:');
	$form->addTextArea('popis', 'Popis:')
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 300 znaku', 300);
	$form->addSubmit('addObor', 'Upravit')
	    ->onClick[] = $this->editOborFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'addOborFormCancel');
	return $form;
    }

    public function editOborFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values = $btn->form->getValues();
	if ($this->oborRepository->isOborInFakulta($values->nazev_oboru, $this->detailFakulty['ID_fakulty'])) {
	    $btn->form->addError('Obor se zadaným názvem na tyhle škole již existuje');
	    return;
	}
	try {
	    $this->oborRepository->getConnection()->exec(
		'UPDATE obor SET Nazev_oboru=?, Delka_studia=?, Forma_studia=?, Titul=?, Zakonceni=?, Popis=? WHERE ID_oboru=?',
		$values->nazev_oboru, $values->delka_studia, $values->forma_studia, $values->titul, $values->zakonceni, $values->popis,
		$this->recordObor['ID_oboru']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	}
    }

    /*
     * pridani predmetu
     */
    protected function createComponentAddPredmetForm()
    {
	$form = new Form;
	$form->addText('zkratka_predmetu', 'Zkratka předmětu:');
	$form->addText('nazev_predmetu', 'Název předmětu:')
	    ->setRequired('Zadejte prosím název předmětu');
	$form->addText('kredity', 'Počet kreditu:')
		->setRequired('Zadejte prosím počet kreditů')
	    ->addCondition($form::FILLED)
	    ->addRule($form::INTEGER, 'Počet kreditu musí být celočíselného typu');
	$form->addText('ukonceni', 'Ukončení:');
	$form->addText('garant', 'Jméno garanta:')
	    ->setRequired('Zadejte prosím jméno garanta');
	$form->addSubmit('addPredmet', 'Přidat')
	    ->onClick[] = $this->addPredmetFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'addPredmetFormCancel');
	return $form;
    }

    public function addPredmetFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	if ($this->predmetRepository->isPredmetInFakulta($values->nazev_predmetu, $this->detailFakulty['ID_fakulty'])) {
	    $btn->form->addError('Zadaný předmět již existuje');
	    return;
	}
	try {
	    $this->predmetRepository->addPredmet($values, $this->detailFakulty['ID_fakulty']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	}
    }

    public function addPredmetFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
    }

    /*
     * editace predmetu
     */
    protected function createComponentEditPredmetForm()
    {
	$form = new Form;
	$form->addText('zkratka_predmetu', 'Zkratka předmětu:');
	$form->addText('nazev_predmetu', 'Název předmětu:')
	    ->setRequired('Zadejte prosím název předmětu');
	$form->addText('kredity', 'Počet kreditu:')
	    ->addCondition($form::FILLED)
	    ->addRule($form::INTEGER, 'Počet kreditu musí být celočíselného typu');
	$form->addText('ukonceni', 'Ukončení:');
	$form->addText('garant', 'Jméno garanta:')
	    ->setRequired('Zadejte prosím jméno garanta');
	$form->addSubmit('addPredmet', 'Upravit')
	    ->onClick[] = $this->editPredmetFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'addPredmetFormCancel');
	return $form;
    }

    public function editPredmetFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	if ($this->predmetRepository->isPredmetInFakulta($values->nazev_predmetu, $this->detailFakulty['ID_fakulty'])) {
	    $btn->form->addError('Zadaný předmět již existuje');
	    return;
	}
	try {
	    $this->predmetRepository->getConnection()->exec(
		'UPDATE predmet SET Zkratka_predmetu=?, Nazev_predmetu=?, Kredity=?, Ukonceni=?, Garant=? WHERE ID_predmetu=?',
		$values->zkratka_predmetu, $values->nazev_predmetu, $values->kredity, $values->ukonceni, $values->garant,
		$this->recordPredmet['ID_predmetu']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	}
    }
    /**
     *	Fuknce pro zachyceni signalu pro pridani polozky do oblibenych 
     */
    public function handleAddFav() {
	    
	    if ( !$this->oblibeneRepository->findAll()
		->where('User_id', $this->getUser()->getId())
		->where('ID_fakulty', $this->detailFakulty->ID_fakulty)->fetch()) {
		$values = array('ID_oblibene' => NULL,
						'User_id' => $this->getUser()->getId(),
						'Nazev_skoly' => NULL,
						'ID_fakulty' => $this->detailFakulty->ID_fakulty);
		$this->oblibeneRepository->addFav($values);
	    }
	    $this->flashMessage('Fakulta byla přidána do oblíbených.', 'success');
    }

    // smazani fakulty
    public function deleteFakulta($id)
    {
	$this->fakultaRepository->deleteFakulta($id);
	$this->flashMessage('Fakulta byla úspěšně odstraněna.', 'success');
	$this->redirect('Skola:');
    }
	
    public function deleteObor($id)
    {
	$this->oborRepository->deleteObor($id);
	$this->flashMessage('Obor byl úspěšně odstraněn.', 'success');
    }

    public function deleteDod($id)
    {
		$this->dodRepository->deleteDod($id);
		$this->flashMessage('DOD byl úspěšně odstraněn.', 'success');
    }

    public function deletePredmet($id)
    {
	$this->predmetRepository->deletePredmet($id);
	$this->flashMessage('Předmět byl úspěšně odstranen', 'success');
    }


    /*
     * pridani fakulty
     */
    protected function createComponentAddFakultaForm() 
    {
	$skoly = $this->skolaRepository->getNamesAsArray();
	$form = new Form;
	$form->addText('nazev_fakulty', 'Název fakulty:')
	    ->setRequired('Zadejte prosím název fakulty');
	$form->addText('adresa_fakulty', 'Adresa:'); 
	$form->addText('email_fakulty', 'Email:');
	$form->addText('web_fakulty', 'Web:'); 
	$form->addText('dekan', 'Dekan:')
	    ->setRequired('Zadejte prosím jméno děkana');
	$form->addSelect('skola', 'Škola:', $skoly)
	    ->setRequired('Zvolte prosím školu, ke které fakulta patří')
	    ->setPrompt('Zvolte školu');
	$form->addUpload('logo_fakulty', 'Logo fakulty:')
	    ->addCondition($form::FILLED)
	    ->addRule($form::MIME_TYPE, 'Obrázek musí být ve formátu jpeg', 'image/jpeg')
	    ->addRule($form::MAX_FILE_SIZE, 'Maximální velikost souboru je 100 KiB', 100*1024);
	$form->addSubmit('addFakulta', 'Přidat')
	    ->onClick[] = $this->addFakultaFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'fakultaFormCancel');
	return $form;
    }

    public function addFakultaFormSubmitted(Nette\Forms\Controls\SubmitButton $btn)
    {
	$values = $btn->form->getValues();
	$row = $this->fakultaRepository->findFakultaInSkola($values->nazev_fakulty, $values->skola);
	if ($row) {
	    $btn->form->addError('Škola již obsahuje fakultu se zadaným jménem.');
	    return;
	}
	try {
	    $this->fakultaRepository->addFakulta($values);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Skola:default');
	}
	catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Skola:default');
	}
    }

    public function fakultaFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect("Skola:");
    }

    /*
     * editace fakulty
     */
    protected function createComponentEditFakultaForm() 
    {
	$skoly = $this->skolaRepository->getNamesAsArray();
	$form = new Form;
	$form->addText('nazev_fakulty', 'Název fakulty:')
	    ->setRequired('Zadejte prosím název fakulty');
	$form->addText('adresa_fakulty', 'Adresa:'); 
	$form->addText('email_fakulty', 'Email:');
	$form->addText('web_fakulty', 'Web:'); 
	$form->addText('dekan', 'Dekan:')
	    ->setRequired('Zadejte prosím jméno děkana');
	$form->addSelect('skola', 'Škola:', $skoly)
	    ->setRequired('Zvolte prosím školu, ke které fakulta patří')
	    ->setPrompt('Zvolte školu');
	$form->addUpload('logo_fakulty', 'Logo fakulty:')
	    ->addCondition($form::FILLED)
	    ->addRule($form::MIME_TYPE, 'Obrázek musí být ve formátu jpeg', 'image/jpeg')
	    ->addRule($form::MAX_FILE_SIZE, 'Maximální velikost souboru je 100 KiB', 100*1024);
	$form->addSubmit('addFakulta', 'Potvrdit změny')
	    ->onClick[] = $this->editFakultaFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'fakultaFormCancel');
	return $form;
    }

    public function editFakultaFormSubmitted(Nette\Forms\Controls\SubmitButton $btn)
    {
	$values = $btn->form->getValues();
	$row = $this->fakultaRepository->findFakultaInSkola($values->nazev_fakulty, $values->skola);
	if ($row) {
	    $btn->form->addError('Škola již obsahuje fakultu se zadaným jménem.');
	    return;
	}
	try {
	    $this->fakultaRepository->getConnection()->exec(
		'UPDATE fakulta SET Nazev_fakulty=?, Adresa_fakulty=?, Email_fakulty=?, Web_fakulty=?, Dekan=?, Nazev_skoly=?
		WHERE ID_fakulty=?',
		$values->nazev_fakulty, $values->adresa_fakulty, $values->email_fakulty, $values->web_fakulty, $values->dekan,
		$values->skola, $this->recordFakulta['ID_fakulty']);
	    if ($values->logo_fakulty != '') {
		$logo = $values->logo_fakulty->toImage();
		$logo->save(__DIR__.'/../../www/images/logos/'.$this->getTable()->max('ID_fakulty').'.jpg', 98);
	    }
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->recordFakulta['ID_fakulty']);
	}
	catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->recordFakulta['ID_fakulty']);
	}
    }

	public function deleteOblibene($id) {
		$this->oblibeneRepository->deleteOblibene($id);
		$this->flashMessage('Položka byla odstraněna ze seznamu oblíbených', 'success');
    }
	
}
