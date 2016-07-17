<?php

use Nette\Application\UI\Form;
/*
 * akce presenter
 */

class AkcePresenter extends BasePresenter
{
    private $akceRepository;
    private $fakultaRepository;
    private $record;
	
    protected function startup()
    {
	parent::startup();
	$this->akceRepository = $this->context->akceRepository;
	$this->fakultaRepository = $this->context->fakultaRepository;
    }

    public function renderDefault()
    {	
	$actions = $this->akceRepository->findAll();
	$actions2 = array();
	foreach ($actions as $action) {
	    $action['Nazev_fakulty'] = $this->fakultaRepository->findAll()->get($action['ID_fakulty'])->Nazev_fakulty;
	    array_push($actions2, $action);
	}
	$this->template->akce = $actions2;
    }

    public function actionAddakce($id_akce) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Akce:');
	}
    }

    public function actionEditakce($id_akce) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Akce:');
	}
	$this->record= $this->akceRepository->findBy(array('ID_akce' => $id_akce))->fetch();
	if (!$this->record) {
	    $this->setView('notFound');
	}
	$this['editAkceForm']->setDefaults(array(
	    'titulek' => $this->record['Titulek'],
	    'popis' => $this->record['Text'],
	    'datum' => $this->record['Kdy'],
	    //'fakulta' => $fakulta//[$this->record['ID_fakulty']]
	    'fakulta' => $this->record['ID_fakulty']
	));
    }
    public function deleteAkce($id)
    {
	$this->akceRepository->deleteAkce($id);
	$this->flashMessage('Akce byla úspěšně odstraněna.', 'success');
	/*$this->redirect('Akce:');*/
    }

    /*
     * pridani akce
     */
    protected function createComponentAddAkceForm()
    {
	$fakulty = array();
	foreach ($this->fakultaRepository->findAll() as $fakulta) {
	    $fakulty[$fakulta->ID_fakulty] = $fakulta->Nazev_fakulty;
	}
	$form = new Form();
	$now = new DateTime();
	$form->addText('titulek', 'Titulek:')
	    ->setRequired('Zadejte prosím název akce');
	$form->addText('datum', 'Datum:')
		->setRequired('Musíte zadat datum')
		->setValue($now->format('Y-m-d H-i-s'));
	$form->addSelect('fakulta', 'Místo podujetí:', $fakulty)
	    ->setPrompt('Vyberte fakultu:')
	    ->setRequired('Musíte zadat místo konání');
	$form->addTextArea('popis', 'Popis:')
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 200 znaku', 200);
	$form->addSubmit('addAkce', 'Přidat')
	    ->onClick[] = $this->addAkceFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'akceFormCancel');
	return $form;
    }

    public function addAkceFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	try {
	    $this->akceRepository->addAkce($values);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Akce:default');
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Akce:default');
	}
    }

    public function akceFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect('Akce:default');
    }

    /*
     * editace akce
     */
    protected function createComponentEditAkceForm()
    {
	$fakulty = array();
	foreach ($this->fakultaRepository->findAll() as $fakulta) {
	    $fakulty[$fakulta->ID_fakulty] = $fakulta->Nazev_fakulty;
	}
	$form = new Form();
	$form->addText('titulek', 'Titulek:')
	    ->setRequired('Zadejte prosím název akce');
	$form->addText('datum', 'Datum:')
		->setRequired('Musíte zadat datum');
	$form->addSelect('fakulta', 'Místo podujetí:', $fakulty)
	    ->setPrompt('Vyberte fakultu:')
	    ->setRequired('Musíte zadat místo konání');
	$form->addTextArea('popis', 'Popis:')
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 200 znaku', 200);
	$form->addSubmit('addAkce', 'Potvrdit změny')
	    ->onClick[] = $this->editAkceFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'akceFormCancel');
	return $form;
    }

    public function editAkceFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	try {
	    $this->akceRepository->getConnection()->exec(
		'UPDATE akce SET Titulek=?, Text=?, Kdy=?, ID_fakulty=? WHERE ID_akce=?',
		$values->titulek, $values->popis, $values->datum, $values->fakulta, $this->record['ID_akce']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Akce:default');
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Akce:default');
	}
    }

}
